#pragma once


#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <queue>
#include <memory>


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
    /// @param socket соке
    /// @param server серве
    explicit WebSocketSession(tcp::socket socket, std::weak_ptr<WebSocketServer> server);
    
    /// @brief начать слушать
    void Start();

    /// @brief положить сообщение в очередь отправки
    /// @param message сообщение
    void SendMessage(std::string message);
    /*
    приняли по ссылке -> пока выполнялась асинхронная отправка, ссылка стала не валидной -> std::string& message не вариант
    приняли по значению, но передали с перемещением -> не сможем отправить больше никому (а нужно минимум получателю и себе)  
    */

    /// @brief проверить подключение клиента
    /// @return клиент кодключён?
    bool Alive() const;
private:
    /// @brief сокет
    websocket::stream<tcp::socket> ws_;
    /// @brief Буфер для хранения сообщений
    boost::beast::flat_buffer buffer_;
    /// @brief очередь сообщений для асинхронной отправки
    std::queue<std::string> responses_q_;
    /// @brief указатель на сервер  
    std::weak_ptr<WebSocketServer> server_;
    /// @brief отправить очередное сообщение
    void WriteMessage();
    /// @brief прочитать сообщение
    void ReadMessage();
};