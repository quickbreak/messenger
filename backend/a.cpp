#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <queue>
#include <unordered_set>

namespace beast = boost::beast;         // Boost.Beast namespace
namespace http = beast::http;           // HTTP from Boost.Beast
namespace websocket = beast::websocket; // WebSocket from Boost.Beast
namespace net = boost::asio;            // Boost.Asio namespace
using tcp = boost::asio::ip::tcp;       // from boost::asio

#define isz(x) (int)x.size()

class WebSocketServer;

// Класс для управления WebSocket соединением
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    explicit WebSocketSession(tcp::socket socket, WebSocketServer *server);
    void start();

    void send_message(std::string message);
    bool alive();
private:
    websocket::stream<tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;    // Буфер для хранения сообщений
    std::queue<std::string> responses_q_; // очередь сообщений для асинхронной отправки
    WebSocketServer *server_;

    void write_message();
    void read_message();
    std::string handle_message(std::string message);
};

class WebSocketServer : public std::enable_shared_from_this<WebSocketServer>
{
public:
    WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
        : acceptor_(ioc, endpoint)
    {
        accept_connection();
    }

    void close_connection(std::shared_ptr<WebSocketSession> session)
    {
        this->sessions_.erase(session);
    }

    void broadcast_message(std::string message, std::shared_ptr<WebSocketSession>sender) {
        for (auto session: sessions_) {
            if (session->alive())
                session->send_message(message);
            else {
                sessions_.erase(session);
            }
        }
    }

private:
    tcp::acceptor acceptor_;
    std::unordered_set<std::shared_ptr<WebSocketSession>> sessions_; // список сессий(клиентов)

    void accept_connection()
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
};

// Реализация методов WebSocketSession
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

std::string WebSocketSession::handle_message(std::string message) 
{
    auto pos = message.find("PING");
    if (pos != std::string::npos && pos == 0) {
        if (message.size() == std::string("PING").size()) { // полагаю, это быстрее, чем сравнивать строки
            // 'PING' -> 'PONG'
            message = "PONG";
        } else {
            // 'PING hello' -> 'hello'
            message = message.substr(pos + std::string("PING").size() + 1);
        }
    }
    else if ((pos = message.find("ping")) != std::string::npos && pos == 0) {
        if (message.size() == std::string("ping").size()) {
            // 'ping' -> 'pong'
            message = "pong";
        } else {
            // 'ping hello' -> 'hello'
            message = message.substr(pos + std::string("ping").size() + 1);
        }
    }
    else {
        // 'hello' -> 'hello'
    }

    return std::move(message);
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
                std::string response = self->handle_message(std::move(received_message));
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

int main()
{
    try
    {
        net::io_context ioc;
        tcp::endpoint endpoint{tcp::v4(), 8080};
        auto server = std::make_shared<WebSocketServer>(ioc, endpoint);
        ioc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}