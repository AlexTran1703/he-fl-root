#include "ws_client.h"

WebSocketClient::WebSocketClient()
    : resolver_(ioc_), ws_(ioc_) {
    setupHandlers();
}

WebSocketClient::~WebSocketClient() {
    close();
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
}

void WebSocketClient::connect(const std::string& host, const std::string& port) {
    auto const results = resolver_.resolve(host, port);
    asio::connect(ws_.next_layer(), results.begin(), results.end());
    ws_.handshake(host, "/");
    
    std::cout << "Connected to " << host << ":" << port << "\n";
    
    io_thread_ = std::thread(&WebSocketClient::runIOContext, this);
    readMessage();
}

void WebSocketClient::sendMessage(const std::string& message) {
    ws_.async_write(asio::buffer(message),
        [this](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            }
        });
}

void WebSocketClient::readMessage() {
    ws_.async_read(buffer_,
        [this](beast::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::string message = beast::buffers_to_string(buffer_.data());
                buffer_.consume(bytes_transferred);
                processMessage(message);
                readMessage();  // Continue reading
            } else {
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        });
}

void WebSocketClient::processMessage(const std::string& message) {
    std::istringstream stream(message);
    std::string command, payload;
    stream >> command;
    std::getline(stream, payload);

    if (handlers_.count(command)) {
        handlers_[command](payload);
    } else {
        std::cerr << "Unknown message type: " << message << std::endl;
    }
}

void WebSocketClient::setupHandlers() {
    handlers_["PONG"] = [](const std::string&) {
        std::cout << "Received PONG response!\n";
    };
    
    handlers_["ECHO"] = [](const std::string& payload) {
        std::cout << "Echoed message: " << payload << "\n";
    };

    handlers_["BROADCAST"] = [](const std::string& payload) {
        std::cout << "Broadcast message: " << payload << "\n";
    };
}

void WebSocketClient::close() {
    beast::error_code ec;
    ws_.close(websocket::close_code::normal, ec);
    if (ec) {
        std::cerr << "Close error: " << ec.message() << std::endl;
    }
}

void WebSocketClient::runIOContext() {
    try {
        ioc_.run();
    } catch (const std::exception& e) {
        std::cerr << "IO Context Error: " << e.what() << std::endl;
    }
}
