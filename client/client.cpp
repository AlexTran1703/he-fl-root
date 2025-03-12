#include "ws/ws_client.h"
#include <iostream>
#include "json.h"
#include "utils.h"
#include "utils_key_generation.h"

using namespace AsymKeyUtils;
using namespace SymKeyUtils;
int main()
{
    Utils::LOG_INFO("Start Client");
    /*JSONHandler jsonHandler;

    jsonHandler.setValue("name", std::string("Alice"));
    jsonHandler.setVector("scores", std::vector<int>{90, 85, 88});
    jsonHandler.setVector("grades", std::vector<std::string>{"A", "B", "A"});
    jsonHandler.setVector("heights", std::vector<double>{5.7, 6.1, 5.9});

    std::cout << "Serialized JSON:\n"
              << jsonHandler.serialize() << std::endl;
    auto output = Utils::runCommand("ls -l");
    std::cout << "COM: " << output << "\r\n";
    std::vector<int> scores = jsonHandler.getVector<int>("scores", {});
    std::cout << "Scores: ";
    for (int score : scores)
        std::cout << score << " ";
    std::cout << std::endl;*/

    try
    {
        WebSocketClient client;
        client.connect("localhost", "8080");
        
        std::string input;
        while (true)
        {
            std::cout << "Enter command (PING, ECHO <message>, BROADCAST <message>, or EXIT): ";
            std::getline(std::cin, input);

            if (input == "EXIT")
            {
                break;
            }
            JSONHandler message;
            message.setValue("type", std::string(input));
            message.setValue("payload", std::string(input));
            client.sendMessage(message);
        }

        client.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
    return 0;
}
