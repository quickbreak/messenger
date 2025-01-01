#pragma once

#include <pqxx/pqxx>
#include <vector>
#include <string>
#include <boost/json.hpp>

namespace chat {
 
    
    struct Message {
        std::string from;
        std::string to;
        std::string content;
        std::string timestamp;
        Message(const std::string from, const std::string to, const std::string content, const std::string timestamp) :
            from(from),
            to(to),
            content(content),
            timestamp(timestamp)
        {};
    };


    struct Chat {
        std::string id;
        std::string user1;
        std::string user2;
        Chat(const std::string id, const std::string user1, const std::string user2) :
            id(id),
            user1(user1),
            user2(user2)
        {};
    };


    class ChatDatabase {
    public:
        explicit ChatDatabase(const std::string& connection_string);
        /// @brief Добавление сообщения в бд
        /// @param from от кого сообщение
        /// @param to кому сообщение
        /// @param content сообщение 
        void InsertMessage(const std::string& from, const std::string& to, const std::string& content);
        /// @brief Получение сообщений между двумя пользователями
        /// @param user1 первый пользователь
        /// @param user2 второй пользователь
        /// @return История сообщений
        boost::json::array GetMessagesBetweenUsers(const std::string& user1, const std::string& user2);
        /// @brief Получение списка чатов пользователя
        /// @param username пользователь
        /// @return Список чатов
        boost::json::array GetChatsList(const std::string& username);
    private:
        /// @brief строка для соединения с бд
        std::string connection_string_;
        /// @brief Преобразование вектора Сообщений в JSON-array
        /// @param messages вектор Сообщений
        /// @return JSON-array
        boost::json::array SerializeMessages(const std::vector<Message>& messages);
        /// @brief Преобразование вектора Чатов в JSON-array
        /// @param chats вектор Чатов
        /// @return JSON-array
        boost::json::array SerializeChats(const std::vector<Chat>& chats);
    };
} // namespace chat
