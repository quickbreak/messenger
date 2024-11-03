#include "WebSocketServer.hpp"
#include "WebSocketSession.hpp"


WebSocketServer::WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
    : acceptor_(ioc, endpoint)
{
    accept_connection();
}


void WebSocketServer::close_connection(std::shared_ptr<WebSocketSession> session)
{
    this->sessions_.erase(session);
}


void WebSocketServer::broadcast_message(std::string message, std::shared_ptr<WebSocketSession>sender) 
{
    for (auto session: sessions_) {
        if (session->alive())
            session->send_message(message);
        else {
            sessions_.erase(session);
        }
    }
}


void WebSocketServer::accept_connection()
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
                session->start();
                for (auto s: this->sessions_) {
                    std::cout << s.get() << ' ';
                }
                std::cout << '\n';
            }
            this->accept_connection(); // Ждем следующие подключения
        }
    );
}
