#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <thread>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class WebSocketClient {
private:
    asio::io_context ioc_;
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    std::thread io_thread_;
    beast::flat_buffer buffer_;

public:
    WebSocketClient(const std::string& host, const std::string& port)
        : resolver_(ioc_), ws_(ioc_) {
        
        auto results = resolver_.resolve(host, port);
        asio::connect(ws_.next_layer(), results.begin(), results.end());
        ws_.handshake(host, "/");

        std::cout << "Connected to " << host << ":" << port << std::endl;

        // Start reading messages asynchronously
        readMessage();

        // Start the IO thread
        io_thread_ = std::thread([this]() { ioc_.run(); });
    }

    ~WebSocketClient() {
        ws_.close(websocket::close_code::normal);
        if (io_thread_.joinable()) io_thread_.join();
    }

    void sendMessage(const std::string& message) {
        ws_.async_write(asio::buffer(message),
            [this](beast::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Send Error: " << ec.message() << std::endl;
                }
            });
    }

private:
    void readMessage() {
        ws_.async_read(buffer_,
            [this](beast::error_code ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::string received = beast::buffers_to_string(buffer_.data());
                    buffer_.consume(bytes_transferred);
                    std::cout << "Server: " << received << std::endl;
                    readMessage();
                } else {
                    std::cerr << "Read Error: " << ec.message() << std::endl;
                }
            });
    }
};

int main() {
    try {
        WebSocketClient client("localhost", "8080");

        std::string message;
        while (true) {
            std::cout << "Enter command (PING, ECHO message, BROADCAST message): ";
            std::getline(std::cin, message);
            if (message == "exit") break;

            client.sendMessage(message);
        }

    } catch (const std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
    return 0;
}
