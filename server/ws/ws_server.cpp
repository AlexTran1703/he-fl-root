#include "ws_server.h"
#include <iostream>

WebSocketSession::WebSocketSession(tcp::socket socket,
                                   std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers)
    : ws_(std::move(socket)), handlers_(std::move(handlers)) {}

void WebSocketSession::start()
{
    ws_.async_accept([self = shared_from_this()](beast::error_code ec)
                     {
if (!ec) {
WebSocketServer::getInstance().addClient(self);
self->readMessage();
} });
}

void WebSocketSession::readMessage()
{
    ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred)
                   {
if (!ec) {
std::string message = beast::buffers_to_string(self->buffer_.data());
self->buffer_.consume(bytes_transferred);
self->processMessage(message);
self->readMessage();
} else {
WebSocketServer::getInstance().removeClient(self);
} });
}

void WebSocketSession::processMessage(const std::string &message)
{
    std::istringstream stream(message);
    std::cout << "MESSAGE:  " << message << std::endl;
    std::string command, payload;
    stream >> command;
    std::cout << "COMMAND:  " << command << std::endl;
    std::cout << "PAYLOAD:  " << payload << std::endl;
    std::getline(stream, payload);

    if (handlers_.count(command))
    {
        handlers_[command](payload, shared_from_this());
    }
    else
    {
        sendMessage("Error: Unknown command '" + command + "'");
    }
}

void WebSocketSession::sendMessage(const std::string &message)
{
    ws_.async_write(asio::buffer(message), [self = shared_from_this()](beast::error_code ec, std::size_t)
                    {
if (ec) {
WebSocketServer::getInstance().removeClient(self);
} });
}

WebSocketServer::WebSocketServer(uint16_t port)
    : acceptor_(ioc_, tcp::endpoint(tcp::v4(), port))
{
    setupHandlers();
    acceptConnection();
    startIOThreads();
}

WebSocketServer &WebSocketServer::getInstance(uint16_t port)
{
    static WebSocketServer instance(port);
    return instance;
}

void WebSocketServer::addClient(std::shared_ptr<WebSocketSession> session)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    active_clients_.insert(session);
}

void WebSocketServer::removeClient(std::shared_ptr<WebSocketSession> session)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    active_clients_.erase(session);
}

void WebSocketServer::broadcastMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto &client : active_clients_)
    {
        client->sendMessage("BROADCAST: " + message);
    }
}

void WebSocketServer::acceptConnection()
{
    acceptor_.async_accept([this](beast::error_code ec, tcp::socket socket)
                           {
        if (!ec)
            std::make_shared<WebSocketSession>(std::move(socket), handlers_)->start();
        acceptConnection(); });
}

void WebSocketServer::setupHandlers()
{
    handlers_["PING"] = [](std::string, std::shared_ptr<WebSocketSession> session)
    {
        session->sendMessage("PONG");
    };
    handlers_["ECHO"] = [](std::string payload, std::shared_ptr<WebSocketSession> session)
    {
        session->sendMessage("ECHO: " + payload);
    };
    handlers_["BROADCAST"] = [this](std::string payload, std::shared_ptr<WebSocketSession>)
    {
        broadcastMessage(payload);
    };
}

void WebSocketServer::startIOThreads()
{
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        threads_.emplace_back([this]()
                              { ioc_.run(); });
    }
}
