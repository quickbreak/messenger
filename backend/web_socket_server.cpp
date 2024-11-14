#include "web_socket_server.hpp"
#include "web_socket_session.hpp"
#include "functions.hpp"

#include <boost/json.hpp>


WebSocketServer::WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
    : acceptor_(ioc, endpoint)
{}


void WebSocketServer::CloseConnection(std::shared_ptr<WebSocketSession> session_ptr)
{
    // можно хранить 2: name, ptr и ptr, name. Тогда всегда будет возможность найти по любому из параметров. А что если их станет больше?
    auto it = this->sessions_.right.find(session_ptr);
    if (it != this->sessions_.right.end()) {
        this->sessions_.right.erase(it);
    }
}


void WebSocketServer::BroadcastMessage(std::string message, std::shared_ptr<WebSocketSession>sender) 
{
    std::cerr << "I am in BroadcastMessage\n";
    for (auto it = sessions_.left.begin(); it != sessions_.left.end();) {
        std::cerr << "Someone`s in the list\n";
        if (it->second->Alive()) {
            std::cerr << "Someone`s Alive\n";
            it->second->SendMessage(message); // Отправляем сообщение активным сессиям
            ++it;
        } else {
            it = sessions_.left.erase(it); // Удаляем неактивные сессии, обновляя итератор            
        }
    }

}


namespace json = boost::json;

void WebSocketServer::SendMessage(std::string request, std::shared_ptr<WebSocketSession>sender) {
    std::cerr << "I am in SendMessage\n";
    json::value parsed_json = json::parse(request);
    json::object obj = parsed_json.as_object();
    std::string received_message = obj["message"].as_string().c_str();
    std::string from = obj["from"].as_string().c_str();
    std::string to = obj["to"].as_string().c_str();
    obj["message"] = functions::GenerateResponse(std::move(received_message));
    //
    // записываем сообщение в бд
    //
    if (to == "all") {
        this->BroadcastMessage(std::move(json::serialize(obj)), sender);
    } else {
        auto it = this->sessions_.left.find(to);
        if (it != this->sessions_.left.end()) { // если клиент ещё доступен, отправляем ему
            it->second->SendMessage(std::move(json::serialize(obj)));
        } else {
            // на нет и суда нет
        }
    } 
}


void WebSocketServer::Authorize(std::string username, std::shared_ptr<WebSocketSession>session_p) {
    this->sessions_.insert({username, session_p});
    std::cerr << "+1 Authorized\n";
}


void WebSocketServer::HandleRequest(std::string request, std::shared_ptr<WebSocketSession>sender) {
    std::cerr << "I am in HandleRequest\n";
    json::value parsed_json = json::parse(request);
    json::object obj = parsed_json.as_object();
    std::string request_type = obj["request_type"].as_string().c_str();
    if (request_type == "auth") {
        this->Authorize(obj["username"].as_string().c_str(), sender);
    } else if (request_type == "msg") {
        this->SendMessage(request, sender);
    } else {
        //
    }
}


void WebSocketServer::AcceptConnection()
{
    acceptor_.async_accept(
        [self = shared_from_this()](beast::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::cout << "Клиент " << " подключился!" << std::endl; // socket.remote_endpoint().address().to_string() - ip,
                // он у всех 127.0.0.1, пока тестируем локально

                // Новая сессия (новый клиент)
                auto session = std::make_shared<WebSocketSession>(std::move(socket), self);
                session->Start();
            }
            self->AcceptConnection(); // Ждем следующие подключения
        }
    );
}
