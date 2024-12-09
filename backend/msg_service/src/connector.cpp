#include "connector.hpp"

#include <iostream>


namespace chat
{
    ChatDatabase::ChatDatabase(const std::string& connection_string)
        : connection_string_(connection_string), db_connection_(nullptr) 
    {}


    void ChatDatabase::EnsureConnection() {
        // если бд была перезапущена, соединение было разорвано
        // тогда его нужно установить заново. Семён Семёныч!!
        if (!this->db_connection_ || !this->db_connection_->is_open()) {
            this->db_connection_ = std::make_unique<pqxx::connection>(this->connection_string_);
            std::cout << "Reconnected to the database.\n";
        }
    }


    void ChatDatabase::InsertMessage(const std::string& from, const std::string& to, const std::string& content) {
        std::string query = R"(
            INSERT INTO messages (from_user, to_user, content, timestamp)
            VALUES ($1, $2, $3, NOW())
        )";
        
        try {
            EnsureConnection();    
            pqxx::work transaction(*db_connection_);
            transaction.exec_params(query, from, to, content);
            transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << '\n';
            throw;
        }
    }


    boost::json::array ChatDatabase::GetMessagesForUser(const std::string& username) {
        std::string query = R"(
            SELECT from_user, to_user, content, timestamp
            FROM messages
            WHERE from_user = $1 OR to_user = $1
            ORDER BY timestamp
        )";

        try {
            EnsureConnection();    
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(query, username);
            transaction.commit();
            std::vector<Message> messages;
            for (const auto& row : result) {
                messages.push_back(Message (
                    row["from_user"].c_str(),
                    row["to_user"].c_str(),
                    row["content"].c_str(),
                    row["timestamp"].c_str()
                ));
            }

            return std::move(SerializeMessages(messages));
        } catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << '\n';
            throw;
        }
    }


    boost::json::array ChatDatabase::GetMessagesBetweenUsers(const std::string& user1, const std::string& user2) {
        std::string query = R"(
            SELECT from_user, to_user, content, timestamp
            FROM messages
            WHERE (from_user = $1 AND to_user = $2)
            OR (from_user = $2 AND to_user = $1)
            ORDER BY timestamp
        )";

        try {
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(query, user1, user2);
            transaction.commit();

            std::vector<Message> messages;
            for (const auto& row : result) {
                messages.push_back({
                    row["from_user"].c_str(),
                    row["to_user"].c_str(),
                    row["content"].c_str(),
                    row["timestamp"].c_str()
                });
            }

            return std::move(SerializeMessages(messages));
        } catch (const std::exception& e) {
            std::cerr << "Database error :" << e.what() << '\n';
            throw;
        }
        
    }


    boost::json::array ChatDatabase::GetChatsList(const std::string& username) {  
        std::string query = R"(
            SELECT DISTINCT from_user, to_user
            FROM messages
            WHERE from_user = $1 OR to_user = $1
        )";      

        try {
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(query, username);
            transaction.commit();

            std::vector<Chat> chats;
            for (const auto& row : result) {
                std::string user2 = (row["from_user"].as<std::string>() == username) ? row["to_user"].as<std::string>() : row["from_user"].as<std::string>();
                chats.push_back(Chat (
                    user2,
                    username,
                    user2
                ));
            }
            return std::move(SerializeChats(chats));
        } catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << '\n';
            throw;
        }
    }


    void ChatDatabase::DeleteMessagesBetweenUsers(const std::string& user1, const std::string& user2) {
        std::string query = R"(
            DELETE FROM messages
            WHERE (from_user = $1 AND to_user = $2)
            OR (from_user = $2 AND to_user = $1)
        )";

        try {
            pqxx::work transaction(*db_connection_);
            transaction.exec_params(query, user1, user2);
            transaction.commit();
            return;
        } catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << '\n';
            throw;
        }
    }


    boost::json::array ChatDatabase::SerializeMessages(const std::vector<Message>& messages) {
        boost::json::array json_array;
        for (const auto& msg : messages) {
            boost::json::object json_msg;
            json_msg["from"] = msg.from;
            json_msg["to"] = msg.to;
            json_msg["message"] = msg.content;
            json_msg["timestamp"] = msg.timestamp;
            json_array.push_back(json_msg);
        }
        return std::move(json_array);
    }


    boost::json::array ChatDatabase::SerializeChats(const std::vector<Chat>& chats) {
        boost::json::array json_array;
        for (const auto& chat : chats) {
            boost::json::object json_msg;
            json_msg["id"] = chat.id;
            json_msg["user2"] = chat.user2;
            json_msg["user1"] = chat.user1;
            json_array.push_back(json_msg);
        }
        return std::move(json_array);
    }
} // namespace chat
