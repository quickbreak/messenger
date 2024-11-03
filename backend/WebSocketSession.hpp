#pragma once


#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <queue>


namespace beast = boost::beast;         // Boost.Beast namespace
namespace websocket = beast::websocket; // WebSocket from Boost.Beast
using tcp = boost::asio::ip::tcp;       // from boost::asio


/// @brief Класс Сервер
class WebSocketServer;


/// @brief Класс взаимодействия с клиентом
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    /// @brief конструктор
    /// @param socket мнемоническое название
    /// @param server мнемоническое название
    explicit WebSocketSession(tcp::socket socket, WebSocketServer *server);
    
    /// @brief начать слушать
    void start();

    /// @brief положить сообщение в очередь отправки
    /// @param message мнемоническое название
    void send_message(std::string message);

    /// @brief проверить подключение клиента
    /// @return клиент кодключён?
    bool alive();
private:
    /// @brief сокет
    websocket::stream<tcp::socket> ws_;
    /// @brief Буфер для хранения сообщений
    boost::beast::flat_buffer buffer_;
    /// @brief очередь сообщений для асинхронной отправки
    std::queue<std::string> responses_q_;
    /// @brief мнемоническое название
    WebSocketServer *server_;
    /// @brief отправить очередное сообщение
    void write_message();
    /// @brief прочитать сообщение
    void read_message();
};