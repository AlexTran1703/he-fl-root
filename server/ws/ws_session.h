#ifndef WEBSOCKET_SESSION_H
#define WEBSOCKET_SESSION_H

#include "ws_server.h"

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
private:
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers_;

public:
    explicit WebSocketSession(tcp::socket socket,
                              std::map<std::string, std::function<void(std::string, std::shared_ptr<WebSocketSession>)>> handlers);

    void start();
    void readMessage();
    void processMessage(const std::string& message);
    void sendMessage(const std::string& message);
};

#endif  // WEBSOCKET_SESSION_H
