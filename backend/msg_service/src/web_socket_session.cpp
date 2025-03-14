#include "web_socket_session.hpp"
#include "web_socket_server.hpp"
#include "functions.hpp"

#include <iostream>


WebSocketSession::WebSocketSession(tcp::socket socket, std::weak_ptr<WebSocketServer> server)
    : ws_(std::move(socket)), buffer_(), server_(std::move(server)) {}


void WebSocketSession::Start()
{
    ws_.async_accept([self = shared_from_this()](beast::error_code ec)
        {
            if (!ec) {
                self->ReadMessage();
            } 
        }
    );
}


bool WebSocketSession::Alive() const {
    return this->ws_.is_open();
}


void WebSocketSession::SendMessage(std::string message)
{
    responses_q_.push(std::move(message)); // кладём очередное сообщение в очередь
    // если до этого очередь была пуста, write_message() остановлена, вызываем её заново
    if (responses_q_.size() == 1)
    {
        WriteMessage();
    }
}


void WebSocketSession::WriteMessage()
{
    if (responses_q_.empty())
    {
        return;
    }
    std::string message = std::move(responses_q_.front());
    responses_q_.pop();
    std::cout << "Будет отправлено: " << message << std::endl;
    ws_.async_write(
        boost::asio::buffer(message),
        // * self - shared_ptr на this, чтобы не произошло
        // уничтожения объекта раньше окончания асинхронной функции !!
        // * типы захватываемых сущностей автоматически выводятся
        // * message = перед std::move(message) обязательно, ведь move надо куда-то вернуть rvalue
        // upd: message = перед std::move(message) не работает, клиенту приходит пустое сообщение
        [self = shared_from_this(), message](boost::beast::error_code ec, std::size_t bytes_transferred)
        {
            // параметры обязательны, не удалось убрать bytes_transferred
            // в лямбда-функции асинхронного метода не стоило захватывать переменную по ссылке (а надо по значению),
            // так как переменная может быть освобождена до того, как асинхронная операция завершится...
            if (!ec) {
                std::cout << "Отправлено: " << message << std::endl;
                self->WriteMessage();
            } else if (ec == boost::system::errc::not_connected || ec == websocket::error::closed || ec == boost::asio::error::operation_aborted) {
                if (auto shared = self->server_.lock()) {                                                                         // CHECK
                    shared->CloseConnection(self);
                }
            } else {
                std::cerr << "Ошибка в WebSocketSession::WriteMessage:async_write. Код: " << ec << " Сообщение: " << ec.message() << std::endl;
            } 
        }
    );
}


void WebSocketSession::ReadMessage()
{
    ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_received)
        {
            if (!ec) {
                std::cout << "Получено сообщение: " << beast::make_printable(self->buffer_.data()) << std::endl;
                std::string request = std::string(
                    boost::asio::buffer_cast<const char*>(self->buffer_.data()),
                    self->buffer_.size()
                );

                // формируем ответ и отправляем его
                // std::string response = functions::HandleRequest(std::move(received_message));
                // self->server_->BroadcastMessage(std::move(response), self); // больше мы response не используем
                if (auto shared = self->server_.lock()) {
                    std::cout << "I am in WebSocketSession::ReadMessage\n";
                    shared->HandleRequest(std::move(request), self);
                    self->buffer_.consume(bytes_received); // очищаем буфер после обработки
                    self->ReadMessage(); // читаем следующее сообщение
                } else {
                    std::cerr << "Ошибка в WebSocketSession::ReadMessage: WebSocketServer сервер недоступен\n";
                }
                
            } else if (ec == boost::system::errc::not_connected || ec == websocket::error::closed) {
                if (auto shared = self->server_.lock()) {
                    shared->CloseConnection(self);
                }
            } else {
                std::cerr << "Ошибка в WebSocketSession::ReadMessage. Код: " << ec << " Сообщение: " << ec.message() << std::endl;
            } 
        }
    );
}
