#include "WebSocketServer.hpp"


int main()
{
    try
    {
        net::io_context ioc;
        tcp::endpoint endpoint{tcp::v4(), 8080};
        auto server = std::make_shared<WebSocketServer>(ioc, endpoint);
        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}

// g++ main.cpp WebSocketServer.cpp WebSocketSession.cpp functions.cpp -o server
