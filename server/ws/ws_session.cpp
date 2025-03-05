#include "ws_session.h"
#include <iostream>

WebSocketSession::WebSocketSession(tcp::socket socket,
                                   std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers)
    : ws_(std::move(socket)), handlers_(std::move(handlers)) {}

void WebSocketSession::start() {
    ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
        if (!ec) {
            WebSocketServer::getInstance().addClient(self);
            self->readMessage();
        }
    });
}

void WebSocketSession::readMessage() {
    ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            std::string message = beast::buffers_to_string(self->buffer_.data());
            self->buffer_.consume(bytes_transferred);
            self->processMessage(message);
            self->readMessage();
        } else {
            WebSocketServer::getInstance().removeClient(self);
        }
    });
}

void WebSocketSession::processMessage(const std::string& message) {
    std::istringstream stream(message);
    std::cout << "MESSAGE:  " << message << std::endl;
    std::string command, payload;
    stream >> command;
    std::cout << "COMMAND:  " << command << std::endl;
    std::cout << "PAYLOAD:  " << payload << std::endl;
    std::getline(stream, payload);

    if (handlers_.count(command)) {
        handlers_[command](payload, shared_from_this());
    } else {
        sendMessage("Error: Unknown command '" + command + "'");
    }
}

void WebSocketSession::sendMessage(const std::string& message) {
    ws_.async_write(asio::buffer(message), [self = shared_from_this()](beast::error_code ec, std::size_t) {
        if (ec) {
            WebSocketServer::getInstance().removeClient(self);
        }
    });
}
