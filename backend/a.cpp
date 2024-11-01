#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <queue>

namespace beast = boost::beast;         // Boost.Beast namespace
namespace http = beast::http;           // HTTP from Boost.Beast
namespace websocket = beast::websocket; // WebSocket from Boost.Beast
namespace net = boost::asio;            // Boost.Asio namespace
using tcp = boost::asio::ip::tcp;       // from boost::asio

#define isz(x) (int)x.size()

class WebSocketServer {
public:
    WebSocketServer(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc, endpoint) {
        accept_connection();
    }

private:
    tcp::acceptor acceptor_;

    void accept_connection() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "Клиент подключился!" << std::endl;

                    // Запуск WebSocket сессии с новым клиентом
                    std::make_shared<WebSocketSession>(std::move(socket))->start();
                }
                accept_connection(); // Ждем следующие подключения
            });
    }

    // Класс для управления WebSocket соединением
    class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
    public:
        explicit WebSocketSession(tcp::socket socket)
            : ws_(std::move(socket)), buffer_() {}

        void start() {
            ws_.async_accept([self = shared_from_this()](beast::error_code ec) {
                if (!ec) {
                    self->read_message();
                }
            });
        }

    private:
        websocket::stream<tcp::socket> ws_;
        boost::beast::flat_buffer buffer_;  // Буфер для хранения сообщений
        std::queue<std::string>responses_q_; // очередь сообщений для асинхронной отправки
        
        // Метод для отправки сообщения клиенту
        void send_message(const std::string message) {
            responses_q_.push(std::move(message)); // кладём очередное сообщение в очередь
            // если до этого очередь была пуста, write_message() остановлена, вызываем её заново
            if (responses_q_.size() == 1) {
                write_message();
            }
        }

        void write_message() {
            if (responses_q_.empty()) {
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
                [self = shared_from_this(), message](boost::beast::error_code ec, std::size_t bytes_transferred) {
                    // параметры обязательны, не удалось убрать bytes_transferred
                    // в лямбда-функции асинхронного метода не стоило захватывать переменную по ссылке (а надо по значению), 
                    // так как переменная может быть освобождена до того, как асинхронная операция завершится...
                    if (!ec) {
                        std::cout << "Отправлено: " << message << std::endl;
                        self->write_message();
                    } else {
                        std::cerr << "Ошибка при отправке сообщения: " << ec.message() << std::endl;
                    }
                }
            );
        }

        void read_message() {
            ws_.async_read(buffer_, [self = shared_from_this()](beast::error_code ec, std::size_t bytes_received) {
                if (!ec) {
                    std::cout << "Получено сообщение: " << beast::make_printable(self->buffer_.data()) << std::endl;
                    std::string received_message = std::string(
                            boost::asio::buffer_cast<const char*>(self->buffer_.data()),
                            self->buffer_.size());
                    
                    std::string response = "";
                    if (received_message.find("PING") != std::string::npos && received_message.find("PING") == 0) {
                        if (received_message.size() == std::string("PING").size()) {
                            // 'PING' -> 'PONG'
                            response = "PONG";
                        }
                        else {
                            // 'PING hello' -> 'hello'
                            response = received_message.substr(received_message.find("PING") + 5);
                        }
                    }
                    else if (received_message.find("ping") != std::string::npos && received_message.find("ping")  == 0) {
                        if (received_message.size() == std::string("ping").size()) {
                            // 'ping' -> 'pong'
                            response = "pong";
                        }
                        else {
                            // 'ping hello' -> 'hello'
                            response = received_message.substr(received_message.find("ping") + 5);
                        }
                    }
                    else {
                        // 'hello' -> 'hello'
                        response = received_message;
                    }
                    self->send_message(std::move(response)); // больше мы response не используем

                    self->buffer_.consume(bytes_received); // очищаем буфер после обработки
                    self->read_message();         // читаем следующее сообщение
                }
            });
        }
    };
};


int main() {
    try {
        net::io_context ioc;
        tcp::endpoint endpoint{tcp::v4(), 8080};
        WebSocketServer server(ioc, endpoint);
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}