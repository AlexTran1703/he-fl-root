#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <set>
#include <mutex>
#include <memory>
#include <map>
#include <functional>

class WebSocketSession;  // Forward declaration

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class WebSocketServer {
public:
    static WebSocketServer& getInstance(uint16_t port = 8080);

    void addClient(std::shared_ptr<WebSocketSession> session);
    void removeClient(std::shared_ptr<WebSocketSession> session);
    void broadcastMessage(const std::string& message);

private:
    asio::io_context ioc_;
    tcp::acceptor acceptor_;
    std::set<std::shared_ptr<WebSocketSession>> active_clients_;
    std::mutex clients_mutex_;
    std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers_;
    std::vector<std::thread> threads_;

    WebSocketServer(uint16_t port);
    void acceptConnection();
    void setupHandlers();
    void startIOThreads();
};

#endif  // WEBSOCKET_SERVER_H
