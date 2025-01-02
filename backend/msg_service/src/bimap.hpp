#pragma once


#include <unordered_map>
#include <memory>
#include "./web_socket_session.hpp"


namespace messenger {
    struct bimap {
    private:
        /// @brief Половина контейнера, в которой ключи таблицы - имена пользователей
        std::unordered_map<std::string, std::shared_ptr<WebSocketSession>>by_usernames_;
        /// @brief Половина контейнера, в которой ключи таблицы - указатели на сессии
        std::unordered_map<std::shared_ptr<WebSocketSession>, std::string>by_sessions_;
    public:
        /// @brief Вставка пары
        /// @param username имя пользователя
        /// @param session_ptr указатель на сессию
        void insert(const std::string& username, const std::shared_ptr<WebSocketSession>& session_ptr);
        /// @brief Удаление пары по имени пользователя
        /// @param username Имя пользователя
        void erase(const std::string& username);
        /// @brief Удаление пары по указателю на сессию
        /// @param session_ptr Указатель на сессию
        void erase(const std::shared_ptr<WebSocketSession>& session_ptr);
        /// @brief Поиск указателя на сессию по имени пользователя
        /// @param username Имя пользователя
        /// @return Указатель на сессию / nullptr
        std::shared_ptr<WebSocketSession> get_session_ptr(const std::string& username) const;
        /// @brief Вывод всех пар
        void print() const;
    };
} // namespace messenger
