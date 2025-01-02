#pragma once


#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <memory>

#include "./bimap.hpp"
#include "./connector.hpp"


namespace beast = boost::beast;         // Boost.Beast namespace
namespace net = boost::asio;            // Boost.Asio namespace
using tcp = boost::asio::ip::tcp;       // from boost::asio
using bimap = messenger::bimap;


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
    void CloseConnection(const std::shared_ptr<WebSocketSession>& session);
    /// @brief обработать запрос
    /// @param request полученный запрос
    /// @param sender отправитель запроса
    void HandleRequest(std::string request, const std::shared_ptr<WebSocketSession>& sender);

private:
    /// @brief Объект взаимодействия с бд
    chat::ChatDatabase db_connector_;
    tcp::acceptor acceptor_;
    /// @brief двусторонний список активных клиентов. Имя пользователя, указатель на объект сессии
    bimap clients_;
    /// @brief отправить клиенту to сообщение message от клиента from
    /// @param received_message полученное сообщение, json-объект как строка
    /// @param sender отправитель запроса
    void SendMessage(const std::string& received_message, const std::shared_ptr<WebSocketSession>& sender);
    /// @brief Добавление клиента в список активных в основном
    /// @param username Имя пользователя 
    /// @param session_p Указатель на сессию
    void Authorize(const std::string& username, const std::shared_ptr<WebSocketSession>& session_p);
    /// @brief отправить клиенту историю одного из его чатов
    /// @param request полученный запрос
    /// @param session_p указатель на объект сессии
    void LoadHistory(const std::string& request, const std::shared_ptr<WebSocketSession>& session_p);
    /// @brief отправить только что вошедшему клиенту множество его чатов
    /// @param username имя пользователя
    /// @param session_p указатель на объект сессии
    void GetChatsList(const std::string& username, const std::shared_ptr<WebSocketSession>& session_p);
    /// @brief закрыть соединение
    /// @param username с каким пользователем
    void CloseConnection(const std::string& username);
};
