#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <set>
#include <mutex>
#include <thread>
#include <map>
#include <functional>
#include <sstream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

// Global variables (thread-safe access needed)
std::set<std::shared_ptr<class WebSocketSession>> active_clients;
std::mutex clients_mutex;

// WebSocket Session (Handles a single client)
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
private:
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers_;

public:
    explicit WebSocketSession(tcp::socket socket,
                              std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers)
        : ws_(std::move(socket)), handlers_(std::move(handlers)) {}

    void start() {
        ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
            if (!ec) {
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    active_clients.insert(self);
                }
                self->readMessage();
            }
        });
    }

    void readMessage() {
        ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::string message = beast::buffers_to_string(self->buffer_.data());
                self->buffer_.consume(bytes_transferred);
                self->processMessage(message);
                self->readMessage();
            } else {
                std::lock_guard<std::mutex> lock(clients_mutex);
                active_clients.erase(self);
            }
        });
    }

    void processMessage(const std::string& message) {
        std::istringstream stream(message);
        std::string command, payload;
        stream >> command;  // Extract the command (first word)
        std::getline(stream, payload); // Extract the rest as payload

        if (handlers_.count(command)) {
            handlers_[command](payload, shared_from_this());  // Call the corresponding handler
        } else {
            sendMessage("Error: Unknown command '" + command + "'");
        }
    }

    void sendMessage(const std::string& message) {
        ws_.async_write(asio::buffer(message), [self = shared_from_this()](beast::error_code ec, std::size_t) {
            if (ec) {
                std::lock_guard<std::mutex> lock(clients_mutex);
                active_clients.erase(self);
            }
        });
    }
};

// WebSocket Server
class WebSocketServer {
private:
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers_;

public:
    WebSocketServer(asio::io_context& ioc, uint16_t port,
                    std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers)
        : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), handlers_(std::move(handlers)) {
        acceptConnection();
    }

private:
    void acceptConnection() {
        acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) {
            if (!ec)
                std::make_shared<WebSocketSession>(std::move(socket), handlers_)->start();
            acceptConnection();
        });
    }
};

// Example function handlers
void handlePing(std::string payload, std::shared_ptr<WebSocketSession> session) {
    session->sendMessage("PONG");
}

void handleEcho(std::string payload, std::shared_ptr<WebSocketSession> session) {
    session->sendMessage("ECHO: " + payload);
}

void handleBroadcast(std::string payload, std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto& client : active_clients) {
        client->sendMessage("BROADCAST: " + payload);
    }
}

int main() {
    try {
        asio::io_context ioc;

        // Define function handlers
        std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers = {
            {"PING", handlePing},
            {"ECHO", handleEcho},
            {"BROADCAST", handleBroadcast}
        };

        WebSocketServer server(ioc, 8080, handlers);
        std::cout << "WebSocket Server running on port 8080...\n";

        std::vector<std::thread> threads;
        for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
            threads.emplace_back([&ioc]() { ioc.run(); });

        for (auto& thread : threads)
            thread.join();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
