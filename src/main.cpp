#include "asio/static_thread_pool.hpp"
#include "asio/thread_pool.hpp"
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <spdlog/spdlog.h>
#include <stop_token>
#include <thread>

using namespace asio;

void handle(ip::tcp::socket& client) {
    std::error_code ec;
    client.write_some(asio::buffer("HTTP/1.0 200 OK\r\n\r\n"));

    ec = client.close(ec);
    if (ec) {
        spdlog::debug("Failed to close (not an error): {}", ec.message());
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

    asio::thread_pool worker_pool(8);

    while (true) {
        ip::tcp::socket client(io);
        ec = acceptor.accept(client, ec);

        if (ec) {
            spdlog::error("Accept failed: {}", ec.message());
            continue;
        }

        asio::post(worker_pool, [client = std::move(client)] mutable { handle(client); });
    }
}
