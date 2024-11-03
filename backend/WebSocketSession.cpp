#include "WebSocketSession.hpp"
#include "WebSocketServer.hpp"
#include "functions.hpp"
#include <iostream>


WebSocketSession::WebSocketSession(tcp::socket socket, WebSocketServer *server)
    : ws_(std::move(socket)), buffer_(), server_(std::move(server)) {}


void WebSocketSession::start()
{
    ws_.async_accept([self = shared_from_this()](beast::error_code ec)
        {
            if (!ec) {
                self->read_message();
            } 
        }
    );
}


bool WebSocketSession::alive() {
    return this->ws_.is_open();
}


void WebSocketSession::send_message(std::string message)
{
    responses_q_.push(std::move(message)); // кладём очередное сообщение в очередь
    // если до этого очередь была пуста, write_message() остановлена, вызываем её заново
    if (responses_q_.size() == 1)
    {
        write_message();
    }
}


void WebSocketSession::write_message()
{
    if (responses_q_.empty())
    {
        return;
    }
    std::string message = std::move(responses_q_.front());
    responses_q_.pop();
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
                self->write_message();
            } else if (ec == boost::system::errc::not_connected || ec == websocket::error::closed) {
                if (self->server_ != NULL) {
                    self->server_->close_connection(self);
                }
            } else {
                std::cerr << "Ошибка при чтении сообщения. Код: " << ec << " Сообщение: " << ec.message() << std::endl;
            } 
        }
    );
}


void WebSocketSession::read_message()
{
    ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_received)
        {
            if (!ec) {
                std::cout << "Получено сообщение: " << beast::make_printable(self->buffer_.data()) << std::endl;
                std::string received_message = std::string(
                    boost::asio::buffer_cast<const char*>(self->buffer_.data()),
                    self->buffer_.size());

                // формируем ответ и отправляем его
                std::string response = functions::handle_message(std::move(received_message));
                self->server_->broadcast_message(std::move(response), self); // больше мы response не используем

                self->buffer_.consume(bytes_received); // очищаем буфер после обработки
                self->read_message(); // читаем следующее сообщение
            } else if (ec == boost::system::errc::not_connected || ec == websocket::error::closed) {
                if (self->server_ != NULL) {
                    self->server_->close_connection(self);
                }
            } else {
                std::cerr << "Ошибка при чтении сообщения. Код: " << ec << " Сообщение: " << ec.message() << std::endl;
            } 
        }
    );
}
