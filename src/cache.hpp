#pragma once

#include "spdlog/spdlog.h"
#include <absl/container/flat_hash_map.h>
#include <exception>
#include <mutex>
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

class BinCache final {
public:
    size_t item_count() noexcept {
        std::shared_lock lock(s_cache_mtx);
        return s_cache.size();
    }

    [[nodiscard]]
    std::optional<std::vector<std::byte>> get_copy(const std::vector<std::byte>& key);

    [[nodiscard]]
    std::optional<ReadLocked<std::vector<std::byte>>> get(const std::vector<std::byte>& key);

    void put(std::vector<std::byte> key, std::vector<std::byte> value) noexcept;

private:
    absl::flat_hash_map<CacheKey, CacheEntry> s_cache { };
    std::shared_mutex s_cache_mtx;
};

class TwoGenCache final {
private:
    BinCache _l1;
    BinCache _l2;
};
