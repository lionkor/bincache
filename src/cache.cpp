#include "cache.hpp"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <limits>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>
#include <vector>

[[nodiscard]]
std::optional<std::vector<std::byte>> BinCache::get_copy(const std::vector<std::byte>& key) {
    std::shared_lock lock(_cache_mtx);
    if (_cache.contains(key)) {
        return _cache.at(key);
    } else {
        return std::nullopt;
    }
}

[[nodiscard]]
std::optional<ReadLocked<std::vector<std::byte>>> BinCache::get(const std::vector<std::byte>& key) {
    std::shared_lock lock(_cache_mtx);
    if (_cache.contains(key)) {
        return ReadLocked<std::vector<std::byte>>(std::move(lock), _cache.at(key));
    } else {
        return std::nullopt;
    }
}

PutStatus BinCache::put(std::vector<std::byte> key, std::vector<std::byte> value) noexcept {
    PutStatus status = PutStatus::Ok;

    // Error_TotalSizeExceeded check
    if (_config.max_total_size_bytes != std::numeric_limits<size_t>::max()
        && _total_values_size + value.size() > _config.max_total_size_bytes) {
        spdlog::error("Total max size configured ({} MiB) reached, tried to add element of size {} which would exceed the threshold (blocking PUT? {})",
            double(_config.max_total_size_bytes) / double(MiB),
            value.size(),
            _config.put_despite_limits ? "no" : "yes");
        status = PutStatus::Error_TotalSizeExceeded;
        if (!_config.put_despite_limits) {
            return status;
        }
    }

    {
        std::unique_lock lock(_cache_mtx);

        // Error_TotalCountExceeded check
        if (_config.max_elements != std::numeric_limits<size_t>::max()
            && _cache.size() + 1 > _config.max_elements) {
            spdlog::error("Total max elements configured ({}) reached, tried to add element which would exceed the threshold (blocking PUT? {})",
                _config.max_elements,
                _cache.size() + 1,
                _config.put_despite_limits ? "no" : "yes");
            status = PutStatus::Error_TotalCountExceeded;
            if (!_config.put_despite_limits) {
                return status;
            }
        }

        if (_cache.contains(key)) {
            _cache.insert_or_assign(std::move(key), std::move(value));
        } else {
            _cache.emplace(std::move(key), std::move(value));
        }
    }
    _total_values_size += value.size();
    return status;
}
