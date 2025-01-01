#include "connector.hpp"

#include <iostream>


namespace chat
{
    ChatDatabase::ChatDatabase(const std::string& connection_string): 
        connection_string_(connection_string)
    {}


    void ChatDatabase::InsertMessage(const std::string& from, const std::string& to, const std::string& content) {
        std::cout << "I`m in ChatDatabase::InsertMessage\n";

        std::string query = R"(
            SELECT id 
            FROM users
            WHERE username = $1
        )";
        try {
            pqxx::connection db_connection(connection_string_);    
            pqxx::work transaction(db_connection);
            pqxx::result from_id_res = transaction.exec_params(query, from);
            pqxx::result to_id_res = transaction.exec_params(query, to);
            std::string from_id = from_id_res[0]["id"].as<std::string>();
            std::string to_id = to_id_res[0]["id"].as<std::string>();
            if (!from_id.empty() && !to_id.empty()) { // Пользователи нашлись
                query = R"(
                    INSERT INTO messages (from_user_id, to_user_id, content, timestamp)
                    VALUES ($1, $2, $3, NOW())
                )";
                transaction.exec_params(query, from_id, to_id, content);
                transaction.commit();
                /*
                Коммит только после выполнения последнего запроса: 
                если данные изменятся между запросами 
                (например, другой пользователь попытается зарегистрироваться с тем же именем),
                атомарность транзакции обеспечит корректность операции.
                */
                return; 
            } else { // Нет нужного (нужных) пользователей
                transaction.abort();
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "ChatDatabase::InsertMessage error: " << e.what() << '\n';
            throw;
        }
    }


    boost::json::array ChatDatabase::GetMessagesBetweenUsers(const std::string& user1, const std::string& user2) {
        std::cout << "I`m in ChatDatabase::GetMessagesBetweenUsers\n";
        
        std::string query = R"(
            SELECT id 
            FROM users
            WHERE username = $1
        )";
        try {
            pqxx::connection db_connection(connection_string_);    
            pqxx::work transaction(db_connection);
            pqxx::result user1_id_res = transaction.exec_params(query, user1);
            pqxx::result user2_id_res = transaction.exec_params(query, user2);
            std::string user1_id = user1_id_res[0]["id"].as<std::string>();
            std::string user2_id = user2_id_res[0]["id"].as<std::string>();
            if (!user1_id.empty() && !user2_id.empty()) { // Пользователи нашлись
                query = R"(
                    SELECT from_user_id, to_user_id, content, timestamp
                    FROM messages
                    WHERE (from_user_id = $1 AND to_user_id = $2)
                    OR (from_user_id = $2 AND to_user_id = $1)
                    ORDER BY timestamp
                )";
                pqxx::result messages_list = transaction.exec_params(query, user1_id, user2_id);
                transaction.commit();
                /*
                Коммит только после выполнения последнего запроса: 
                если данные изменятся между запросами 
                (например, другой пользователь попытается зарегистрироваться с тем же именем),
                атомарность транзакции обеспечит корректность операции.
                */
                std::vector<Message> messages;
                for (const auto& row : messages_list) {
                    messages.push_back({
                        row["from_user_id"].c_str() == user1_id ? user1 : user2,
                        row["to_user_id"].c_str() == user1_id ? user1 : user2,
                        row["content"].c_str(),
                        row["timestamp"].c_str()
                    });
                }
                return std::move(SerializeMessages(messages));
            } else { // Нет нужного (нужных) пользователей
                transaction.abort();
                std::vector<Message> messages;
                return std::move(SerializeMessages(messages));
            }
        } catch (const std::exception& e) {
            std::cerr << "ChatDatabase::GetMessagesBetweenUsers error :" << e.what() << '\n';
            throw;
        }
    }


    boost::json::array ChatDatabase::GetChatsList(const std::string& username) {
        std::cout << "I`m in ChatDatabase::GetChatsList\n";
        
        std::string query = R"(
            SELECT id 
            FROM users
            WHERE username = $1
        )";
        try {
            pqxx::connection db_connection(connection_string_);    
            pqxx::work transaction(db_connection);
            pqxx::result user1_id_res = transaction.exec_params(query, username);
            std::string user1_id = user1_id_res[0]["id"].as<std::string>(); 
            if (!user1_id.empty()) { // Пользователь нашёлся
                query = R"(
                    SELECT DISTINCT from_user_id, to_user_id
                    FROM messages
                    WHERE from_user_id = $1 OR to_user_id = $1
                )";
                pqxx::result chats_list = transaction.exec_params(query, user1_id);
                /*
                Коммит только после выполнения последнего запроса: 
                если данные изменятся между запросами 
                (например, другой пользователь попытается зарегистрироваться с тем же именем),
                атомарность транзакции обеспечит корректность операции.
                */  
                query = R"(
                    SELECT username 
                    FROM users
                    WHERE id = $1
                )";
                std::string user2_id, user2;
                pqxx::result user2_result;
                std::vector<Chat> chats;
                for (const auto& row : chats_list) {
                    user2_id = row["from_user_id"].as<std::string>() != user1_id ? row["from_user_id"].as<std::string>() : row["to_user_id"].as<std::string>();
                    user2_result = transaction.exec_params(query, user2_id);
                    user2 = user2_result[0]["username"].as<std::string>();
                    chats.push_back(Chat (
                        user2,
                        username,
                        user2
                    ));
                }
                transaction.commit();
                return std::move(SerializeChats(chats));
            } else { // Нет нужного пользователя
                transaction.abort();
                std::vector<Chat> chats;
                return std::move(SerializeChats(chats));
            }
        } catch (const std::exception& e) {
            std::cerr << "ChatDatabase::GetChatsList error: " << e.what() << '\n';
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
