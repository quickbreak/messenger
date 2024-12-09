#include "web_socket_server.hpp"
#include "web_socket_session.hpp"
#include "functions.hpp"

#include <boost/json.hpp>


WebSocketServer::WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
    : acceptor_(ioc, endpoint), db_connector_("dbname=mydb user=postgres password=mypassword host=db port=5432")
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
    // std::cerr << "I am in BroadcastMessage\n";
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
    // std::cerr << "I am in SendMessage\n";
    json::value parsed_json = json::parse(request);
    json::object obj = parsed_json.as_object();
    std::string received_message = obj["message"].as_string().c_str();
    std::cout << "Собственно сообщение: " << received_message << '\n';
    std::string from = obj["from"].as_string().c_str();
    std::string to = obj["to"].as_string().c_str();
    //
    // записываем сообщение в бд
    this->db_connector_.InsertMessage(from, to, received_message);
    //
    obj["message"] = functions::GenerateResponse(std::move(received_message));
    if (to == "all") {
        this->BroadcastMessage(std::move(json::serialize(obj)), sender);
    } else {
        // отправить адресату 
        auto it = this->sessions_.left.find(to);
        if (it != this->sessions_.left.end()) { // если клиент ещё доступен, отправляем ему
            it->second->SendMessage(json::serialize(obj));
        } else {
            // на нет и суда нет
        }
        /* фронт отобразит сообщение у адресанта, хватит пересылать туда-сюда
        it = this->sessions_.left.find(from);
        if (it != this->sessions_.left.end()) { // если клиент ещё доступен, отправляем ему
            it->second->SendMessage(json::serialize(obj));
        } else {
            // на нет и суда нет
        }
        */
    } 
}


void WebSocketServer::Authorize(std::string username, std::shared_ptr<WebSocketSession>session_p) {
    this->sessions_.insert({username, session_p});
    std::cerr << "+1 Authorized\n";
}

void WebSocketServer::LoadHistory(const std::string& request, std::shared_ptr<WebSocketSession>session_p) {
    json::value parsed_json = json::parse(request);
    json::object req_obj = parsed_json.as_object();
    std::string username = req_obj["username"].as_string().c_str();
    std::string chat_with = req_obj["chat_with"].as_string().c_str();

    boost::json::object obj;
    obj["request_type"] = "history";
    obj["chat_with"] = chat_with;
    obj["history"] = this->db_connector_.GetMessagesBetweenUsers(username, chat_with);

    // отправить клиенту username историю его общенияя с chat_with
    auto it = this->sessions_.left.find(username);
    if (it != this->sessions_.left.end()) { // если клиент ещё доступен, отправляем ему
        it->second->SendMessage(json::serialize(obj));
    }
}

void WebSocketServer::GetChatsList(const std::string username, std::shared_ptr<WebSocketSession>session_p) {
    boost::json::object obj;
    obj["request_type"] = "chats";
    std::cout << "I`m in WSServer::GetChatsList\n";
    obj["chats"] = this->db_connector_.GetChatsList(username);
    std::cout << "I`m in WSServer::GetChatsList. Chats got\n";
    
    // отправить клиенту username список его чатов
    auto it = this->sessions_.left.find(username);
    if (it != this->sessions_.left.end()) { // если клиент ещё доступен, отправляем ему
        std::cout << "I`m in WSServer::GetChatsList. Клиент ещё доступен, отправляем ему\n";
        it->second->SendMessage(json::serialize(obj));
    }
    std::cerr << "I`m in WSServer::GetChatsList. Клиент не доступен / его нет в списке\n";
}

void WebSocketServer::HandleRequest(std::string request, std::shared_ptr<WebSocketSession>sender) {
    // std::cerr << "I am in HandleRequest\n";
    json::value parsed_json = json::parse(request);
    json::object obj = parsed_json.as_object();
    std::string request_type = obj["request_type"].as_string().c_str();
    if (request_type == "auth") {
        this->Authorize(obj["username"].as_string().c_str(), sender);
        this->GetChatsList(obj["username"].as_string().c_str(), sender);
    } else if (request_type == "history") {
        this->LoadHistory(request, sender);
    } else if (request_type == "msg") {
       this->SendMessage(request, sender);
    }
}


void WebSocketServer::AcceptConnection()
{
    acceptor_.async_accept(
        [self = shared_from_this()](beast::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::cout << "Клиент " << " подключился!" << std::endl;

                // Новая сессия (новый клиент)
                auto session = std::make_shared<WebSocketSession>(std::move(socket), self);
                session->Start();
            }
            self->AcceptConnection(); // Ждем следующие подключения
        }
    );
}
