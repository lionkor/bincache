#include "cache.hpp"
#include "defer.hpp"
#include <algorithm>
#include <array>
#include <asio/buffer.hpp>
#include <asio/error_code.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp> // IWYU pragma: keep
#include <asio/write.hpp> // IWYU pragma: keep
#include <asio/socket_base.hpp>
#include <asio/thread_pool.hpp>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <spdlog/spdlog.h>
#include <system_error>
#include <utility>
#include <vector>

using namespace asio;

// Because we're SO nice!!!
void gracefully_close(ip::tcp::socket& client) {
    asio::error_code ec;
    ec = client.shutdown(asio::socket_base::shutdown_both, ec);
    if (ec) {
        spdlog::debug("Failed to shutdown (not an error): {}", ec.message());
    }
    ec = client.close(ec);
    if (ec) {
        spdlog::debug("Failed to close (not an error): {}", ec.message());
    }
}

static constexpr size_t OPERATION_SIZE = 3;
static constexpr size_t MAX_CONCURRENCY = 8;
static constexpr size_t KEY_HEADER_SIZE = sizeof(uint16_t);
static constexpr size_t VALUE_HEADER_SIZE = sizeof(uint32_t);
static constexpr std::array<std::byte, OPERATION_SIZE> OPERATION_GET = { std::byte('G'), std::byte('E'), std::byte('T') };
static constexpr std::array<std::byte, OPERATION_SIZE> OPERATION_PUT = { std::byte('P'), std::byte('U'), std::byte('T') };

static BinCache s_cache { };

static inline void do_get(ip::tcp::socket& client, std::vector<std::byte>& key) {
    asio::error_code ec;
    // PERF: This is the copying variant.
    auto maybe_data = s_cache.get_copy(key);
    if (maybe_data.has_value()) {
        uint32_t data_size = maybe_data.value().size();

        if (std::endian::native == std::endian::big) {
            // swap to LE :^)
            data_size = std::byteswap(data_size);
        }

        // reinterpret_cast is needed here, i dont want to do a memcpy.
        // NOLINTNEXTLINE
        auto* data_size_ptr = reinterpret_cast<std::byte*>(&data_size);

        asio::write(client, asio::buffer(data_size_ptr, sizeof(data_size)), ec);
        if (ec) {
            spdlog::error("Failed to write value size header: {}", ec.message());
            return;
        }

        asio::write(client, asio::buffer(maybe_data.value()), ec);
        if (ec) {
            spdlog::error("Failed to write value: {}", ec.message());
            return;
        }
    }
}

static inline void do_put(ip::tcp::socket& client, std::vector<std::byte>& key) {
    asio::error_code ec;
    std::array<std::byte, VALUE_HEADER_SIZE> value_size_header{};
    asio::read(client, asio::buffer(value_size_header), ec);
    if (ec) {
        spdlog::error("Malformed packet: Failed to read value size header: {}", ec.message());
        return;
    }

    uint32_t value_size{};
    static_assert(sizeof(value_size) == value_size_header.size(), "key size buffer has the wrong byte count");
    std::memcpy(&value_size, value_size_header.data(), sizeof(value_size));

    if constexpr (std::endian::native == std::endian::big) {
        // do a little swapping
        value_size = std::byteswap(value_size);
    }

    std::vector<std::byte> value(value_size);
    asio::read(client, asio::buffer(value), ec);
    if (ec) {
        spdlog::error("Malformed packet: Failed to read value: {}", ec.message());
        return;
    }

    // PERF: these are two copies, key can be moved if needed
    PutStatus status = s_cache.put(key, value);
    switch (status) {
    case PutStatus::Ok:
        // nice!
        break;
    case PutStatus::Error_TotalSizeExceeded:
        break;
    case PutStatus::Error_TotalCountExceeded:
        break;
    }

    // spdlog::info("Put {} bytes for key \"{}\"", value_size, std::string_view(reinterpret_cast<char*>(key.data()), key.size()));
    //  for PUT and GET we return the cached value

    // write the same header, no need to recompute it
    asio::write(client, asio::buffer(value_size_header), ec);
    if (ec) {
        spdlog::error("Failed to write value size header back after PUT: {}", ec.message());
        return;
    }
    // write the value akin to get
    asio::write(client, asio::buffer(value), ec);
    if (ec) {
        spdlog::error("Failed to write value back after PUT: {}", ec.message());
        return;
    }
}

// Format:
// 0. The operation "GET" or "PUT" without the terminating null byte
// 1. A 2-byte unsigned int, little endian, denoting the key size
// 2. Key as a byte[] of length read from 1.
// 3. For PUT, a 4-byte unsigned int, little endian, denoting data size
// 4. For PUT, data of size read from 3.
void handle(ip::tcp::socket& client) {
    const DeferredAction defer_graceful_close([&client] { gracefully_close(client); });

    std::error_code ec;

    std::array<std::byte, OPERATION_SIZE + KEY_HEADER_SIZE> header{};
    asio::read(client, asio::buffer(header), ec);
    if (ec) {
        // this isn't "malformed packet" because we're not sure it's even a
        // whole packet, so we just say we failed
        spdlog::error("Failed to read header: {}", ec.message());
        return;
    }

    spdlog::debug("read {} bytes of header", header.size());

    const auto operation = std::span<std::byte, OPERATION_SIZE>(header.begin(), header.begin() + OPERATION_SIZE);
    const auto key_size_header = std::span<std::byte, KEY_HEADER_SIZE>(header.begin() + OPERATION_SIZE, header.begin() + OPERATION_SIZE + KEY_HEADER_SIZE);

    // we only care if it is PUT or not.
    bool is_put = std::ranges::equal(OPERATION_PUT, operation);

    uint16_t key_size { 0 };
    static_assert(sizeof(key_size) == key_size_header.size(), "key size buffer has the wrong byte count");
    std::memcpy(&key_size, key_size_header.data(), sizeof(key_size));

    if constexpr (std::endian::native == std::endian::big) {
        // do a little swapping
        key_size = std::byteswap(key_size);
    }

    std::vector<std::byte> key(key_size);
    asio::read(client, asio::buffer(key), ec);
    if (ec) {
        spdlog::error("Malformed packet: Failed to read {} key bytes: {}", key_size, ec.message());
        return;
    }

    if (is_put) {
        do_put(client, key);
    } else {
        do_get(client, key);
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    asio::io_context io;
    ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::make_address("127.0.0.1"), 3900));

    asio::error_code ec;
    ec = acceptor.set_option(ip::tcp::acceptor::reuse_address(true), ec);
    if (ec)
        spdlog::warn("Failed to set REUSEADDR, ignoring");

    acceptor.listen();
    spdlog::info("Listening on [{}]:{}", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

    asio::thread_pool worker_pool(MAX_CONCURRENCY);

    while (true) {
        ip::tcp::socket client(io);
        ec = acceptor.accept(client, ec);

        if (ec) {
            spdlog::error("Accept failed: {}", ec.message());
            continue;
        }

        // this is ~25% faster than just calling handle() directly here.
        asio::post(worker_pool, [client = std::move(client)] mutable { handle(client); });
    }
}
