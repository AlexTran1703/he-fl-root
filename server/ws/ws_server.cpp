#include "ws_server.h"
#include <iostream>
#include "../../utils/json.h"
WebSocketSession::WebSocketSession(tcp::socket socket,
                                   std::map<std::string, std::function<void(JSONHandler&, std::shared_ptr<WebSocketSession>)>> handlers)
    : ws_(std::move(socket)), handlers_(std::move(handlers)) {}

void WebSocketSession::start() {
    ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
    if (!ec) {
        WebSocketServer::getInstance().addClient(self);
        self->readMessage();
    } });
}

void WebSocketSession::readMessage()
{
    ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
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
    JSONHandler message_json;
    if(!message_json.parse(message)){
        Utils::LOG_ERROR("Error when parsing json message");
        return;
    }
    std::string command, payload;
    command = message_json.getValue<std::string>("type");
    payload = message_json.getValue<std::string>("payload");
    if (handlers_.count(command))
    {
        handlers_[command](message_json, shared_from_this());
    }
    else
    {
        Utils::LOG_ERROR("Error: Unknown command");
        sendMessage("Error: Unknown command '" + command + "'");
    }
}

void WebSocketSession::sendMessage(const std::string &message)
{
    ws_.async_write(asio::buffer(message), [self = shared_from_this()](beast::error_code ec, std::size_t) {
        if (ec) {
            WebSocketServer::getInstance().removeClient(self);
        } });
}

void WebSocketSession::sendMessage(const JSONHandler &message_json)
{
    std::string message = message_json.serialize();
    std::cout << "Message Server send to Client: " << message << std::endl;
    ws_.async_write(asio::buffer(message), [self = shared_from_this()](beast::error_code ec, std::size_t) {
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

void WebSocketServer::broadcastMessage(const JSONHandler &message)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto &client : active_clients_)
    {
        client->sendMessage(message.serialize());
    }
}

void WebSocketServer::shutdownMessage(const JSONHandler &message)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto &client : active_clients_)
    {
        client->sendMessage(message.serialize());
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
    handlers_["PING"] = [](JSONHandler& payload, std::shared_ptr<WebSocketSession> session)
    {
        payload.setValue("type", std::string("PONG"));
        session->sendMessage(payload);
    };
    handlers_["ECHO"] = [](JSONHandler& payload, std::shared_ptr<WebSocketSession> session)
    {
        session->sendMessage(payload);
    };
    handlers_["BROADCAST"] = [this](JSONHandler& payload, std::shared_ptr<WebSocketSession>)
    {
        broadcastMessage(payload);
    };
    handlers_["VECTOR"] = [](JSONHandler& payload, std::shared_ptr<WebSocketSession>)
    {
        std::vector<double> scores = payload.getVector<double>("values");
        double sum = 0;
        for (double score : scores){
            std::cout << score << " ";
            sum += score;
        }
        std::cout << std::endl;
        std::cout << "Total sum: " << sum << std::endl;
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
