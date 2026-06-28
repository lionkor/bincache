#pragma once

#include "consts.hpp"
#include <absl/container/flat_hash_map.h>
#include <chrono>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

using CacheKey = std::vector<std::byte>;
using CacheEntry = std::vector<std::byte>;

template <typename H>
H AbslHashValue(H h, const CacheKey& key) {
    return H::combine_contiguous(std::move(h), key.data(), key.size());
}

template <typename T>
struct ReadLocked final {
    const T& value;

    explicit ReadLocked(std::shared_lock<std::shared_mutex>&& lock, const T& value)
        : value(value)
        , _lock(std::move(lock)) {
        if (!_lock.owns_lock()) {
            throw std::logic_error("Lock must be owned & locked for ReadLocked to take it");
        }
    }

private:
    std::shared_lock<std::shared_mutex> _lock;
};

using namespace consts::sizes;
struct CacheConfig final {
    size_t max_total_size_bytes = 1 * GiB;
    size_t max_elements = 10'000;
    // CONTINUE: Set this back to false, change main to call `get` instead of `get_copy` and
    // see if that changes performance characteristics
    bool put_despite_limits = false;

    // TODO
    std::optional<std::chrono::steady_clock::duration> max_age = std::nullopt;
};

enum class PutStatus {
    Ok,
    Error_TotalSizeExceeded,
    Error_TotalCountExceeded,
};

class BinCache final {
public:
    explicit BinCache(const CacheConfig& config = { })
        : _config(config) { }

    size_t item_count() const noexcept {
        std::shared_lock lock(_cache_mtx);
        return _cache.size();
    }

    [[nodiscard]]
    std::optional<std::vector<std::byte>> get_copy(const std::vector<std::byte>& key);

    // CAREFUL: As the type indicates, this holds a READ lock on the cache.
    [[nodiscard]]
    std::optional<ReadLocked<std::vector<std::byte>>> get(const std::vector<std::byte>& key);

    [[nodiscard]]
    PutStatus put(std::vector<std::byte> key, std::vector<std::byte> value) noexcept;

private:
    absl::flat_hash_map<CacheKey, CacheEntry> _cache { };
    mutable std::shared_mutex _cache_mtx;
    CacheConfig _config;

    size_t _total_values_size = 0;
};

class TwoGenCache final {
private:
    BinCache _l1;
    BinCache _l2;
};
