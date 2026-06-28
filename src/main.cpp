#include <asio/ip/address.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <spdlog/spdlog.h>

using namespace asio;

void handle_client() {

}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    asio::io_context io;

    auto ep = ip::tcp::endpoint(ip::make_address("127.0.0.1"), 3900);
    auto acceptor = ip::tcp::acceptor(io, ep);
    asio::error_code ec;
    ec = acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec) {
        // ignore if this fails, just warn
        spdlog::warn("Failed to set REUSEADDR, ignoring");
    }

    ec = acceptor.listen(socket_base::max_listen_connections, ec);
    if (ec) {
        spdlog::error("Failed to listen on {}: {}", ep.address().to_string(), ec.message());
        return 1;
    }

    spdlog::info("Listening on [{}]:{}", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

    ip::tcp::endpoint client_ep;
    //acceptor.async_accept(ep, handle_client);
}
