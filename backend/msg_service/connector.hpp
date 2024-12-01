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
    };

    class ChatDatabase {
    public:
        explicit ChatDatabase(const std::string& connection_string);

        void InsertMessage(const std::string& from, const std::string& to, const std::string& content);
        boost::json::array GetMessagesForUser(const std::string& username);
        boost::json::array GetMessagesBetweenUsers(const std::string& user1, const std::string& user2);

        // Удаление всех сообщений между двумя пользователями (опционально)
        void DeleteMessagesBetweenUsers(const std::string& user1, const std::string& user2);

    private:
        pqxx::connection db_connection_;
        /// @brief преобразовать вектор Сообщений в JSON-array
        /// @param messages вектор Сообщений
        /// @return JSON-array
        boost::json::array SerializeMessages(const std::vector<Message>& messages);
    };
} // namespace chat
