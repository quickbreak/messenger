#include "web_socket_server.hpp"


int main()
{
    try
    {
        net::io_context ioc;
        tcp::endpoint endpoint{tcp::v4(), 8080};
        auto server = std::make_shared<WebSocketServer>(ioc, endpoint);
        server->AcceptConnection();
        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка в main: " << e.what() << std::endl;
    }
}

 
// g++ main.cpp web_socket_server.cpp web_socket_session.cpp functions.cpp -I/opt/vcpkg/installed/x64-linux/include -L/opt/vcpkg/installed/x64-linux/lib -lboost_json -o server