#include "cache.hpp"

[[nodiscard]]
std::optional<std::vector<std::byte>> BinCache::get_copy(const std::vector<std::byte>& key) {
    std::shared_lock lock(s_cache_mtx);
    if (s_cache.contains(key)) {
        return s_cache.at(key);
    } else {
        return std::nullopt;
    }
}

[[nodiscard]]
std::optional<ReadLocked<std::vector<std::byte>>> BinCache::get(const std::vector<std::byte>& key) {
    std::shared_lock lock(s_cache_mtx);
    if (s_cache.contains(key)) {
        return ReadLocked<std::vector<std::byte>>(std::move(lock), s_cache.at(key));
    } else {
        return std::nullopt;
    }
}

void BinCache::put(std::vector<std::byte> key, std::vector<std::byte> value) noexcept {
    std::unique_lock lock(s_cache_mtx);
    // PERF: emplace is better if not exists, probably
    s_cache.insert_or_assign(std::move(key), std::move(value));
}
