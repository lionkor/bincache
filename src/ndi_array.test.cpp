#include "ndi_array.hpp"

#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

// --- type constraint ---

static_assert(std::is_trivially_default_constructible_v<int>);
static_assert(std::is_trivially_default_constructible_v<std::byte>);

// --- construction: size-based ---

TEST(NdiArrayTest, DefaultConstructZeroSize) {
    NdiArray<int> arr(0);
    EXPECT_EQ(arr.size(), 0u);
    EXPECT_EQ(arr.span().size(), 0u);
    auto cspan = std::as_const(arr).span();
    EXPECT_EQ(cspan.size(), 0u);
}

TEST(NdiArrayTest, DefaultConstructNonZeroSize) {
    constexpr size_t kSize = 5;
    NdiArray<int> arr(kSize);
    EXPECT_EQ(arr.size(), kSize);
    EXPECT_EQ(arr.span().size(), kSize);
    EXPECT_EQ(arr.data(), arr.span().data());
}

TEST(NdiArrayTest, DefaultConstructNonZeroSizeConst) {
    constexpr size_t kSize = 3;
    NdiArray<int> arr(kSize);
    const auto& carr = std::as_const(arr);
    EXPECT_EQ(carr.size(), kSize);
    EXPECT_EQ(carr.span().size(), kSize);
    EXPECT_EQ(carr.data(), carr.span().data());
}

// --- construction: copy ---

TEST(NdiArrayTest, CopyConstructEmpty) {
    NdiArray<int> arr(0);
    NdiArray<int> copy(arr);
    EXPECT_EQ(copy.size(), 0u);
    EXPECT_NE(copy.data(), arr.data()); // independence (or both null)
}

TEST(NdiArrayTest, CopyConstructPreservesValues) {
    NdiArray<int> arr(4);
    arr.span()[0] = 10;
    arr.span()[1] = 20;
    arr.span()[2] = 30;
    arr.span()[3] = 40;

    NdiArray<int> copy(arr);
    EXPECT_EQ(copy.size(), arr.size());
    EXPECT_NE(copy.data(), arr.data());
    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(copy.span()[i], arr.span()[i]);
    }
}

TEST(NdiArrayTest, CopyConstructIsIndependent) {
    NdiArray<int> arr(3);
    arr.span()[0] = 1;
    arr.span()[1] = 2;
    arr.span()[2] = 3;

    NdiArray<int> copy(arr);
    copy.span()[0] = 99;

    EXPECT_EQ(arr.span()[0], 1);
    EXPECT_EQ(copy.span()[0], 99);
    EXPECT_EQ(arr.span()[1], 2);
    EXPECT_EQ(copy.span()[1], 2);
}

// --- construction: span-based (mutable) ---

TEST(NdiArrayTest, ConstructFromMutableSpanEmpty) {
    std::vector<int> src;
    std::span<int> s(src);
    NdiArray<int> arr(s);
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, ConstructFromMutableSpanPreservesValues) {
    std::vector<int> src { 5, 6, 7 };
    std::span<int> s(src);
    NdiArray<int> arr(s);
    EXPECT_EQ(arr.size(), src.size());
    EXPECT_NE(arr.data(), src.data());
    for (size_t i = 0; i < src.size(); ++i) {
        EXPECT_EQ(arr.span()[i], src[i]);
    }
}

TEST(NdiArrayTest, ConstructFromMutableSpanIsIndependent) {
    std::vector<int> src { 1, 2, 3 };
    std::span<int> s(src);
    NdiArray<int> arr(s);
    arr.span()[0] = 100;
    EXPECT_EQ(src[0], 1);
    EXPECT_EQ(arr.span()[0], 100);
    src[0] = 200;
    EXPECT_EQ(arr.span()[0], 100);
}

// --- construction: span-based (const) ---

TEST(NdiArrayTest, ConstructFromConstSpanEmpty) {
    const std::vector<int> src;
    std::span<const int> s(src);
    NdiArray<int> arr(s);
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, ConstructFromConstSpanPreservesValues) {
    const std::vector<int> src { 11, 12, 13, 14 };
    std::span<const int> s(src);
    NdiArray<int> arr(s);
    EXPECT_EQ(arr.size(), src.size());
    EXPECT_NE(arr.data(), src.data());
    for (size_t i = 0; i < src.size(); ++i) {
        EXPECT_EQ(arr.span()[i], src[i]);
    }
}

TEST(NdiArrayTest, ConstructFromConstSpanIsIndependent) {
    const std::vector<int> src { 7, 8, 9 };
    std::span<const int> s(src);
    NdiArray<int> arr(s);
    arr.span()[0] = 99;
    EXPECT_EQ(src[0], 7);
    EXPECT_EQ(arr.span()[0], 99);
}

// --- copy assignment ---

TEST(NdiArrayTest, CopyAssignSameSize) {
    NdiArray<int> a(3);
    a.span()[0] = 10;
    a.span()[1] = 20;
    a.span()[2] = 30;

    NdiArray<int> b(3);
    b = a;
    EXPECT_EQ(b.size(), a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(b.span()[i], a.span()[i]);
    }
    EXPECT_NE(b.data(), a.data());
}

TEST(NdiArrayTest, CopyAssignDifferentSize) {
    NdiArray<int> a(2);
    a.span()[0] = 5;
    a.span()[1] = 6;

    NdiArray<int> b(5);
    b = a;
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(b.span()[0], 5);
    EXPECT_EQ(b.span()[1], 6);
}

TEST(NdiArrayTest, CopyAssignLarger) {
    NdiArray<int> a(5);
    a.span()[0] = 0;
    a.span()[1] = 1;
    a.span()[2] = 2;
    a.span()[3] = 3;
    a.span()[4] = 4;

    NdiArray<int> b(2);
    b = a;
    EXPECT_EQ(b.size(), 5u);
    EXPECT_EQ(b.span()[4], 4);
}

TEST(NdiArrayTest, CopyAssignSelfAssignment) {
    NdiArray<int> arr(3);
    arr.span()[0] = 1;
    arr.span()[1] = 2;
    arr.span()[2] = 3;

    // NOLINTNEXTLINE
    arr = arr; // self-assignment
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr.span()[0], 1);
    EXPECT_EQ(arr.span()[1], 2);
    EXPECT_EQ(arr.span()[2], 3);
    EXPECT_NE(arr.data(), nullptr);
}

TEST(NdiArrayTest, CopyAssignFromEmpty) {
    NdiArray<int> empty(0);
    NdiArray<int> arr(3);
    arr = empty;
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, CopyAssignToEmpty) {
    NdiArray<int> arr(3);
    arr.span()[0] = 7;
    NdiArray<int> empty(0);
    empty = arr;
    EXPECT_EQ(empty.size(), 3u);
    EXPECT_EQ(empty.span()[0], 7);
}

TEST(NdiArrayTest, CopyAssignChain) {
    NdiArray<int> a(1);
    a.span()[0] = 42;
    NdiArray<int> b(1);
    NdiArray<int> c(1);
    c = b = a;
    EXPECT_EQ(b.span()[0], 42);
    EXPECT_EQ(c.span()[0], 42);
}

// --- move construction ---

TEST(NdiArrayTest, MoveConstructTransfersOwnership) {
    NdiArray<int> arr(4);
    arr.span()[0] = 1;
    arr.span()[1] = 2;
    arr.span()[2] = 3;
    arr.span()[3] = 4;

    auto* old_data = arr.data();
    size_t old_size = arr.size();

    NdiArray<int> moved(std::move(arr));
    EXPECT_EQ(moved.data(), old_data);
    EXPECT_EQ(moved.size(), old_size);
    EXPECT_EQ(moved.span()[0], 1);
    EXPECT_EQ(moved.span()[1], 2);
    EXPECT_EQ(moved.span()[2], 3);
    EXPECT_EQ(moved.span()[3], 4);

    // Source is in a valid-but-unspecified state. Convention is nullptr/0.
    EXPECT_EQ(arr.data(), nullptr);
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, MoveConstructFromEmpty) {
    NdiArray<int> arr(0);
    NdiArray<int> moved(std::move(arr));
    EXPECT_EQ(moved.size(), 0u);
    EXPECT_EQ(arr.size(), 0u);
    EXPECT_EQ(arr.data(), nullptr);
}

// --- move assignment ---

TEST(NdiArrayTest, MoveAssignTransfersOwnership) {
    NdiArray<int> a(3);
    a.span()[0] = 10;
    a.span()[1] = 20;
    a.span()[2] = 30;

    auto* old_data = a.data();
    size_t old_size = a.size();

    NdiArray<int> b(1);
    b = std::move(a);
    EXPECT_EQ(b.data(), old_data);
    EXPECT_EQ(b.size(), old_size);
    EXPECT_EQ(b.span()[0], 10);
    EXPECT_EQ(b.span()[1], 20);
    EXPECT_EQ(b.span()[2], 30);
}

TEST(NdiArrayTest, MoveAssignSourceBecomesEmpty) {
    NdiArray<int> a(3);
    a.span()[0] = 1;
    NdiArray<int> b(2);
    b = std::move(a);
    EXPECT_EQ(a.data(), nullptr);
    EXPECT_EQ(a.size(), 0u);
}

TEST(NdiArrayTest, MoveAssignSelfAssignment) {
    NdiArray<int> arr(3);
    arr.span()[0] = 1;
    arr.span()[1] = 2;
    arr.span()[2] = 3;

    // NOLINTNEXTLINE
    arr = std::move(arr); // self-move-assignment
    EXPECT_NE(arr.data(), nullptr);
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr.span()[0], 1);
    EXPECT_EQ(arr.span()[1], 2);
    EXPECT_EQ(arr.span()[2], 3);
}

TEST(NdiArrayTest, MoveAssignIntoEmpty) {
    NdiArray<int> a(3);
    a.span()[0] = 7;
    NdiArray<int> b(0);
    b = std::move(a);
    EXPECT_EQ(b.size(), 3u);
    EXPECT_EQ(b.span()[0], 7);
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.data(), nullptr);
}

TEST(NdiArrayTest, MoveAssignFromEmpty) {
    NdiArray<int> a(0);
    NdiArray<int> b(5);
    b = std::move(a);
    EXPECT_EQ(b.size(), 0u);
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.data(), nullptr);
}

// --- span() ---

TEST(NdiArrayTest, SpanMutableAllowsMutation) {
    NdiArray<int> arr(3);
    auto s = arr.span();
    s[0] = 100;
    s[1] = 200;
    s[2] = 300;
    EXPECT_EQ(arr.span()[0], 100);
    EXPECT_EQ(arr.span()[1], 200);
    EXPECT_EQ(arr.span()[2], 300);
}

TEST(NdiArrayTest, SpanConstReflectsData) {
    NdiArray<int> arr(3);
    arr.span()[0] = 5;
    arr.span()[1] = 6;
    arr.span()[2] = 7;
    const auto& carr = std::as_const(arr);
    auto cs = carr.span();
    EXPECT_EQ(cs[0], 5);
    EXPECT_EQ(cs[1], 6);
    EXPECT_EQ(cs[2], 7);
}

TEST(NdiArrayTest, SpanZeroSize) {
    NdiArray<int> arr(0);
    EXPECT_EQ(arr.span().size(), 0u);
    EXPECT_EQ(arr.span().data(), arr.data());
}

TEST(NdiArrayTest, SpanConstZeroSize) {
    const NdiArray<int> arr(0);
    EXPECT_EQ(arr.span().size(), 0u);
    EXPECT_EQ(arr.span().data(), arr.data());
}

// --- data() ---

TEST(NdiArrayTest, DataMutablePointsToArray) {
    NdiArray<int> arr(2);
    arr.data()[0] = 10;
    arr.data()[1] = 20;
    EXPECT_EQ(arr.span()[0], 10);
    EXPECT_EQ(arr.span()[1], 20);
}

TEST(NdiArrayTest, DataConstPointsToArray) {
    NdiArray<int> arr(2);
    arr.span()[0] = 33;
    arr.span()[1] = 44;
    const auto& carr = std::as_const(arr);
    EXPECT_EQ(carr.data()[0], 33);
    EXPECT_EQ(carr.data()[1], 44);
}

TEST(NdiArrayTest, DataReturnsSameAsSpanData) {
    NdiArray<int> arr(5);
    EXPECT_EQ(arr.data(), arr.span().data());
    const auto& carr = std::as_const(arr);
    EXPECT_EQ(carr.data(), carr.span().data());
}

// --- size() ---

TEST(NdiArrayTest, SizeReturnsCorrectValue) {
    NdiArray<int> a(0);
    EXPECT_EQ(a.size(), 0u);
    NdiArray<int> b(1);
    EXPECT_EQ(b.size(), 1u);
    NdiArray<int> c(1024);
    EXPECT_EQ(c.size(), 1024u);
}

TEST(NdiArrayTest, SizeAfterCopyAssign) {
    NdiArray<int> a(5);
    NdiArray<int> b(10);
    b = a;
    EXPECT_EQ(b.size(), 5u);
}

TEST(NdiArrayTest, SizeAfterMoveAssign) {
    NdiArray<int> a(7);
    NdiArray<int> b(1);
    b = std::move(a);
    EXPECT_EQ(b.size(), 7u);
}

// --- clear() ---

TEST(NdiArrayTest, ClearDefaultValue) {
    NdiArray<int> arr(4);
    arr.span()[0] = 9;
    arr.span()[1] = 9;
    arr.span()[2] = 9;
    arr.span()[3] = 9;

    arr.clear();
    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(arr.span()[i], 0); // int{} is 0
    }
}

TEST(NdiArrayTest, ClearCustomValue) {
    NdiArray<int> arr(5);
    arr.span()[0] = 1;
    arr.span()[1] = 2;

    arr.clear(42);
    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_EQ(arr.span()[i], 42);
    }
}

TEST(NdiArrayTest, ClearOnEmpty) {
    NdiArray<int> arr(0);
    // Must not crash.
    arr.clear();
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, ClearCustomValueOnEmpty) {
    NdiArray<int> arr(0);
    arr.clear(99);
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, ClearAfterCopyKeepsIndependence) {
    NdiArray<int> a(3);
    a.span()[0] = 1;
    a.span()[1] = 2;
    a.span()[2] = 3;
    NdiArray<int> b(a);

    a.clear(5);
    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a.span()[i], 5);
    }
    EXPECT_EQ(b.span()[0], 1);
    EXPECT_EQ(b.span()[1], 2);
    EXPECT_EQ(b.span()[2], 3);
}

// --- non-int type (std::byte) ---

TEST(NdiArrayTest, ByteTypeConstruction) {
    NdiArray<std::byte> arr(3);
    EXPECT_EQ(arr.size(), 3u);
    arr.span()[0] = std::byte { 0xAA };
    arr.span()[1] = std::byte { 0xBB };
    arr.span()[2] = std::byte { 0xCC };

    EXPECT_EQ(arr.span()[0], std::byte { 0xAA });
    EXPECT_EQ(arr.span()[1], std::byte { 0xBB });
    EXPECT_EQ(arr.span()[2], std::byte { 0xCC });
}

TEST(NdiArrayTest, ByteTypeClearDefault) {
    NdiArray<std::byte> arr(2);
    arr.span()[0] = std::byte { 0xFF };
    arr.span()[1] = std::byte { 0xEE };
    arr.clear();
    EXPECT_EQ(arr.span()[0], std::byte { 0 });
    EXPECT_EQ(arr.span()[1], std::byte { 0 });
}

TEST(NdiArrayTest, ByteTypeClearCustom) {
    NdiArray<std::byte> arr(2);
    arr.span()[0] = std::byte { 0x00 };
    arr.clear(std::byte { 0xAB });
    EXPECT_EQ(arr.span()[0], std::byte { 0xAB });
    EXPECT_EQ(arr.span()[1], std::byte { 0xAB });
}

TEST(NdiArrayTest, ByteTypeCopyAndMove) {
    NdiArray<std::byte> a(2);
    a.span()[0] = std::byte { 0x01 };
    a.span()[1] = std::byte { 0x02 };

    NdiArray<std::byte> b(a);
    EXPECT_EQ(b.span()[0], std::byte { 0x01 });
    EXPECT_EQ(b.span()[1], std::byte { 0x02 });

    NdiArray<std::byte> c(std::move(b));
    EXPECT_EQ(c.span()[0], std::byte { 0x01 });
    EXPECT_EQ(c.span()[1], std::byte { 0x02 });
    EXPECT_EQ(b.data(), nullptr);
    EXPECT_EQ(b.size(), 0u);
}

// --- large array stress test ---

TEST(NdiArrayTest, LargeArray) {
    constexpr size_t kSize = 100'000;
    NdiArray<int> arr(kSize);
    EXPECT_EQ(arr.size(), kSize);

    // Write and read back
    for (size_t i = 0; i < kSize; ++i) {
        arr.span()[i] = static_cast<int>(i);
    }
    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(arr.span()[i], static_cast<int>(i));
    }

    // Copy large
    NdiArray<int> copy(arr);
    EXPECT_EQ(copy.size(), kSize);
    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(copy.span()[i], static_cast<int>(i));
    }
    EXPECT_NE(copy.data(), arr.data());

    // Move large
    auto* old_data = arr.data();
    NdiArray<int> moved(std::move(arr));
    EXPECT_EQ(moved.data(), old_data);
    EXPECT_EQ(moved.size(), kSize);
    EXPECT_EQ(arr.data(), nullptr);
    EXPECT_EQ(arr.size(), 0u);
}

TEST(NdiArrayTest, LargeArrayClear) {
    constexpr size_t kSize = 50'000;
    NdiArray<int> arr(kSize);
    for (size_t i = 0; i < kSize; ++i) {
        arr.span()[i] = 1;
    }
    arr.clear(7);
    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(arr.span()[i], 7);
    }
}

// --- integration-style: real-world usage pattern ---

TEST(NdiArrayTest, RealWorldUsagePattern) {
    // Simulate what cache.cpp does: create from vector, copy-construct, move
    const auto key = std::vector<std::byte> {
        std::byte { 'h' }, std::byte { 'e' }, std::byte { 'l' }, std::byte { 'l' }, std::byte { 'o' }
    };
    const auto value = std::vector<std::byte> {
        std::byte { 'w' }, std::byte { 'o' }, std::byte { 'r' }, std::byte { 'l' }, std::byte { 'd' }
    };

    // Construct from const span (what cache does with CacheEntry)
    NdiArray<std::byte> entry { std::span<const std::byte>(value) };
    EXPECT_EQ(entry.size(), value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        EXPECT_EQ(entry.span()[i], value[i]);
    }

    // Copy (what get_copy does: return _cache.at(key))
    NdiArray<std::byte> copy(entry);
    EXPECT_EQ(copy.size(), entry.size());
    for (size_t i = 0; i < entry.size(); ++i) {
        EXPECT_EQ(copy.span()[i], entry.span()[i]);
    }

    // Move (what put does: std::move(value))
    auto* old_data = entry.data();
    NdiArray<std::byte> moved(std::move(entry));
    EXPECT_EQ(moved.data(), old_data);
    EXPECT_EQ(entry.data(), nullptr);
    EXPECT_EQ(entry.size(), 0u);
}

// --- const correctness compile-time checks ---

TEST(NdiArrayTest, ConstSpanIsReadOnly) {
    NdiArray<int> arr(3);
    arr.span()[0] = 10;
    const auto& carr = std::as_const(arr);
    // carr.span() returns std::span<const int>, so this compiles:
    const int v = carr.span()[0];
    EXPECT_EQ(v, 10);
}

TEST(NdiArrayTest, ConstDataIsReadOnly) {
    NdiArray<int> arr(1);
    arr.span()[0] = 42;
    const auto& carr = std::as_const(arr);
    const int v = carr.data()[0];
    EXPECT_EQ(v, 42);
}
