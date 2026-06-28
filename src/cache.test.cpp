#include "cache.hpp"

#include <gtest/gtest.h>

#include <cstddef>

TEST(BinCacheTest, PutAndGetCopy) {
    BinCache cache;
    EXPECT_EQ(cache.item_count(), 0);

    const auto key = std::vector<std::byte> {
        std::byte { 'h' }, std::byte { 'e' }, std::byte { 'l' }, std::byte { 'l' }, std::byte { 'o' }
    };
    const auto value = std::vector<std::byte> {
        std::byte { 'w' }, std::byte { 'o' }, std::byte { 'r' }, std::byte { 'l' }, std::byte { 'd' }
    };

    cache.put(key, value);
    EXPECT_EQ(cache.item_count(), 1);

    auto result = cache.get_copy(key);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, value);
}
