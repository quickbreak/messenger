#include "web_socket_server.hpp"
#include "web_socket_session.hpp"


#include <boost/json.hpp>


namespace json = boost::json;


const char* db_name = std::getenv("DB_NAME");
const char* db_user = std::getenv("DB_USER");
const char* db_password = std::getenv("DB_PASSWORD");
const char* db_host = std::getenv("DB_HOST");
const char* db_port = std::getenv("DB_PORT");
const std::string connection_string = "dbname=" + std::string(db_name) + " user=" + std::string(db_user) + " password=" + std::string(db_password) + " host=" + std::string(db_host) + " port=" + std::string(db_port);


WebSocketServer::WebSocketServer(net::io_context &ioc, tcp::endpoint endpoint)
    : acceptor_(ioc, endpoint), db_connector_(connection_string)
{}


void WebSocketServer::CloseConnection(const std::shared_ptr<WebSocketSession>& session_ptr)
{
    this->clients_.erase(session_ptr);
}


void WebSocketServer::CloseConnection(const std::string& username)
{
    this->clients_.erase(username);
}


void WebSocketServer::SendMessage(const std::string& request, const std::shared_ptr<WebSocketSession>& sender) {
    std::cout << "I am in WebSocketServer::SendMessage\n";
    json::value parsed_json = json::parse(request);
    json::object obj = parsed_json.as_object();
    std::string received_message = obj["message"].as_string().c_str();
    std::cout << "Собственно сообщение: " << received_message << '\n';
    std::string from = obj["from"].as_string().c_str();
    std::string to = obj["to"].as_string().c_str();
    // записываем сообщение в бд
    try {
        this->db_connector_.InsertMessage(from, to, received_message);
    }
    catch (...) {
        std::cerr << "Ошибка в WebSocketServer::SendMessage\n";
        std::cerr << "Ошибка при попытке записать сообщение в бд\n";
    }
    // отправляем адресату сообщение
    auto session_ptr = this->clients_.get_session_ptr(to);
    if (session_ptr != nullptr) { // если пользователь онлайн, отправляем
        std::cout << "I`m in WSServer::SendMessage. Клиент ещё доступен, отправляем ему\n";
        session_ptr->SendMessage(json::serialize(obj));
    } else {
        std::cerr << "I`m in WSServer::SendMessage. Клиент не доступен / его нет в списке\n";
    }
}


void WebSocketServer::Authorize(const std::string& username, const std::shared_ptr<WebSocketSession>& session_ptr) {
    this->CloseConnection(username);
    this->clients_.insert(username, session_ptr);

    std::cout << "+1 Authorized\n";
    std::cout << "Список пользователей:\n";
    this->clients_.print();
}


void WebSocketServer::LoadHistory(const std::string& request, const std::shared_ptr<WebSocketSession>& session_p) {
    json::value parsed_json = json::parse(request);
    json::object req_obj = parsed_json.as_object();
    std::string username = req_obj["username"].as_string().c_str();
    std::string chat_with = req_obj["chat_with"].as_string().c_str();

    boost::json::object obj;
    obj["request_type"] = "history";
    obj["chat_with"] = chat_with;
    obj["history"] = this->db_connector_.GetMessagesBetweenUsers(username, chat_with);

    // отправить клиенту username историю его общения с chat_with
    auto session_ptr = this->clients_.get_session_ptr(username);
    if (session_ptr != nullptr) { // если пользователь онлайн, отправляем
        std::cout << "I`m in WSServer::LoadHistory. Клиент ещё доступен, отправляем ему\n";
        session_ptr->SendMessage(json::serialize(obj));
    } else {
        std::cerr << "I`m in WSServer::LoadHistory. Клиент не доступен / его нет в списке\n";
    }
}


void WebSocketServer::GetChatsList(const std::string& username, const std::shared_ptr<WebSocketSession>& session_p) {
    boost::json::object obj;
    obj["request_type"] = "chats";
    std::cout << "I`m in WSServer::GetChatsList\n";
    obj["chats"] = this->db_connector_.GetChatsList(username);
    std::cout << "I`m in WSServer::GetChatsList. Chats got\n";
    
    // отправить клиенту username список его чатов
    auto session_ptr = this->clients_.get_session_ptr(username);
    if (session_ptr != nullptr) { // если пользователь онлайн, отправляем
        std::cout << "I`m in WSServer::GetChatsList. Клиент ещё доступен, отправляем ему\n";
        session_ptr->SendMessage(json::serialize(obj));
    } else {
        std::cerr << "I`m in WSServer::GetChatsList. Клиент не доступен / его нет в списке\n";
    }
}


void WebSocketServer::HandleRequest(std::string request, const std::shared_ptr<WebSocketSession>& sender) {
    std::cout << "I am in WebSocketServer::HandleRequest\n";
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
