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

void WebSocketClient::sendMessage(const JSONHandler& message_json) {
    std::string message = message_json.serialize();
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
                Utils::LOG_ERROR("Read error: " + ec.message());
                std::cerr << "Server close connection" << std::endl;
                JSONHandler payload_exit;
                payload_exit.setValue("type", std::string("EXIT"));
                handlers_["EXIT"](payload_exit);
            }
        });
}

void WebSocketClient::processMessage(const std::string& message) {
    std::istringstream stream(message);
    std::cout << "MESSAGE:  " << message << std::endl;
    JSONHandler message_json;
    if(!message_json.parse(message)){
        Utils::LOG_ERROR("Error when parsing json message");
        return;
    }
    std::string command, payload;
    command = message_json.getValue<std::string>("type");
    payload = message_json.getValue<std::string>("payload");
    if (handlers_.count(command)) {
        handlers_[command](message_json);
    } else {
        std::cerr << "Unknown message type: " << message << std::endl;
    }
}

void WebSocketClient::setupHandlers() {
    handlers_["PONG"] = [](JSONHandler& payload) {
        std::cout << "PONG: " << payload.serialize() << "\n";
        std::cout << "Received PONG response!\n";
    };
    
    handlers_["ECHO"] = [](JSONHandler& payload) {
        std::cout << "Echoed message: " << payload.serialize() << "\n";
    };

    handlers_["BROADCAST"] = [](JSONHandler& payload) {
        std::cout << "Broadcast message: " << payload.serialize() << "\n";
    };
    handlers_["EXIT"] = [](JSONHandler& payload) {
        Utils::LOG_WARNING("Server close connection or session");
        exit(1);
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
