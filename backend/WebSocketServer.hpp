#pragma once


#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <unordered_set>


namespace beast = boost::beast;         // Boost.Beast namespace
namespace net = boost::asio;            // Boost.Asio namespace
using tcp = boost::asio::ip::tcp;       // from boost::asio


class WebSocketSession;


class WebSocketServer : public std::enable_shared_from_this<WebSocketServer>
{
public:
    /// @brief конструктор сервера
    WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint);
    /// @brief закрыть соединение
    /// @param session какое соединение
    void close_connection(std::shared_ptr<WebSocketSession> session);
    /// @brief отправить сообщение всем
    /// @param message какое сообщение
    /// @param sender от кого
    void broadcast_message(std::string message, std::shared_ptr<WebSocketSession>sender);

private:
    tcp::acceptor acceptor_;
    std::unordered_set<std::shared_ptr<WebSocketSession>> sessions_; // список сессий(клиентов)
    /// @brief ждать новое подключение
    void accept_connection();
};