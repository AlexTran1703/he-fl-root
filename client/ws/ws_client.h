#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <thread>
#include <map>
#include <functional>
#include "../../utils/json.h"
#include "../../utils/utils.h"
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class WebSocketClient {
private:
    asio::io_context ioc_;
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::map<std::string, std::function<void(JSONHandler&)>> handlers_;
    std::thread io_thread_;

public:
    WebSocketClient();
    ~WebSocketClient();
    
    void connect(const std::string& host, const std::string& port);
    void sendMessage(const std::string& message);
    void sendMessage(const JSONHandler& message);
    void readMessage();
    void processMessage(const std::string& message);
    void close();

private:
    void runIOContext();
    void setupHandlers();
};

#endif  // WEBSOCKET_CLIENT_H
