#include "web_socket_server.hpp"
#include "web_socket_session.hpp"


WebSocketServer::WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
    : acceptor_(ioc, endpoint)
{
    AcceptConnection();
}


void WebSocketServer::CloseConnection(std::shared_ptr<WebSocketSession> session)
{
    this->sessions_.erase(session);
}


void WebSocketServer::BroadcastMessage(std::string message, std::shared_ptr<WebSocketSession>sender) 
{
    for (auto session: sessions_) {
        if (session->Alive())
            session->SendMessage(message);
        else {
            sessions_.erase(session);
        }
    }
}


void WebSocketServer::AcceptConnection()
{
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::cout << "Клиент " << " подключился!" << std::endl; // socket.remote_endpoint().address().to_string() - ip,
                // он у всех 127.0.0.1, пока тестируем локально

                // Новая сессия (новый клиент)
                auto session = std::make_shared<WebSocketSession>(std::move(socket), this);
                this->sessions_.insert(session);
                session->Start();
                for (auto s: this->sessions_) {
                    std::cout << s.get() << ' ';
                }
                std::cout << '\n';
            }
            this->AcceptConnection(); // Ждем следующие подключения
        }
    );
}
