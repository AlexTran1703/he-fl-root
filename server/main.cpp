#include "ws/ws_server.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        WebSocketServer::getInstance(8080);
        std::cout << "WebSocket Server running on port 8080...\n";
        
        std::this_thread::sleep_for(std::chrono::hours(1)); // Keeps main thread alive

    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
    return 0;
}
