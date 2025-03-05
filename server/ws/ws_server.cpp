#include "ws_server.h"
#include "ws_session.h"
#include <iostream>

WebSocketServer::WebSocketServer(uint16_t port) 
    : acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)) {
    setupHandlers();
    acceptConnection();
    startIOThreads();
}

WebSocketServer& WebSocketServer::getInstance(uint16_t port) {
    static WebSocketServer instance(port);
    return instance;
}

void WebSocketServer::addClient(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    active_clients_.insert(session);
}

void WebSocketServer::removeClient(std::shared_ptr<WebSocketSession> session) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    active_clients_.erase(session);
}

void WebSocketServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& client : active_clients_) {
        client->sendMessage("BROADCAST: " + message);
    }
}

void WebSocketServer::acceptConnection() {
    acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket) {
        if (!ec)
            std::make_shared<WebSocketSession>(std::move(socket), handlers_)->start();
        acceptConnection();
    });
}

void WebSocketServer::setupHandlers() {
    handlers_["PING"] = [](std::string, std::shared_ptr<WebSocketSession> session) {
        session->sendMessage("PONG");
    };
    handlers_["ECHO"] = [](std::string payload, std::shared_ptr<WebSocketSession> session) {
        session->sendMessage("ECHO: " + payload);
    };
    handlers_["BROADCAST"] = [this](std::string payload, std::shared_ptr<WebSocketSession>) {
        broadcastMessage(payload);
    };
}

void WebSocketServer::startIOThreads() {
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        threads_.emplace_back([this]() { ioc_.run(); });
    }
}
