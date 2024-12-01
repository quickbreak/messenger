#pragma once


#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <memory>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include "connector.hpp"


namespace beast = boost::beast;         // Boost.Beast namespace
namespace net = boost::asio;            // Boost.Asio namespace
using tcp = boost::asio::ip::tcp;       // from boost::asio


class WebSocketSession;


class WebSocketServer : public std::enable_shared_from_this<WebSocketServer>
{
public:
    /// @brief конструктор сервера
    WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint);
    /// @brief ждать новое подключение
    void AcceptConnection();
    /// @brief закрыть соединение
    /// @param session какое соединение
    void CloseConnection(std::shared_ptr<WebSocketSession> session);
    /// @brief обработать запрос
    /// @param request полученный запрос
    /// @param sender отправитель запроса
    void HandleRequest(std::string request, std::shared_ptr<WebSocketSession>sender);

private:
    chat::ChatDatabase db_connector_;
    tcp::acceptor acceptor_;
    /// @brief список активных сессий(клиентов). Имя пользователя -> указатель на объект сессии
    // std::unordered_map<std::string, std::shared_ptr<WebSocketSession>> sessions_;
    /// @brief двусторонний список актиынх сессий(клиентов). Имя пользователя, указатель на объект сессии
    boost::bimap<
    boost::bimaps::unordered_set_of<std::string>,
    boost::bimaps::unordered_set_of<std::shared_ptr<WebSocketSession>>> sessions_;
    /// @brief отправить сообщение всем
    /// @param message какое сообщение
    /// @param sender от кого
    void BroadcastMessage(std::string message, std::shared_ptr<WebSocketSession>sender);
    /// @brief отправить клиенту to сообщение message от клиента from
    /// @param received_message полученное сообщение, json-объект как строка
    /// @param sender отправитель запроса
    void SendMessage(std::string received_message, std::shared_ptr<WebSocketSession>sender);
    /// @brief добавление клиента в список активных в основном
    /// @param request 
    /// @param session_p 
    void Authorize(std::string request, std::shared_ptr<WebSocketSession>session_p);
    /// @brief отправить клиенту историю одного из его чатов
    /// @param request полученный запрос
    /// @param session_p указатель на объект сессии
    void LoadHistory(const std::string& request, std::shared_ptr<WebSocketSession>session_p);
    /// @brief отправить только что вошедшему клиенту множество его чатов
    /// @param username имя пользователя
    /// @param session_p указатель на объект сессии
    void GetChatsList(std::string username, std::shared_ptr<WebSocketSession>session_p);
};