#include "auth.hpp"


namespace auth {
    AuthService::AuthService(const std::string& conn_str) : 
        conn_(conn_str) 
    {}


    bool AuthService::Authenticate(const std::string& username, const std::string& password) {
        try {
            const std::string query = R"(
            SELECT password 
            FROM users
            WHERE username = $1
            )";
            
            pqxx::work txn(conn_);
            pqxx::result result = txn.exec_params(query, username);
            txn.commit();

            if (result.empty()) { // Пользователь не найден
                return false;
            }

            std::string stored_hash = result[0]["password"].as<std::string>();

            return VerifyPassword(password, stored_hash);

        } catch (const std::exception& e) {
            throw std::runtime_error("Ошибка при аутентификации: " + std::string(e.what()));
        }
    };


    bool AuthService::Register(const std::string& username, const std::string& password) {
        try {
            const std::string check_query = R"(
            SELECT username 
            FROM users
            WHERE username = $1
            )";

            pqxx::work txn(conn_);
            pqxx::result result = txn.exec_params(check_query, username);
            txn.commit();

            if (result.empty()) { // логин не занят, регистрируем нового пользователя
                const std::string register_query = R"(
                INSERT INTO users 
                (username, password)
                VALUES ($1, $2)
                )";

                pqxx::work txn(conn_);
                pqxx::result result = txn.exec_params(register_query, username, password);
                txn.commit();
                
                return true; 
            } else { // логин занят
                return false;
            }

        } catch (const std::exception& e) {
            throw std::runtime_error("Ошибка при регистрации: " + std::string(e.what()));
        }
    };


    bool AuthService::Find(const std::string& username) {
        try {
            const std::string check_query = R"(
            SELECT username 
            FROM users
            WHERE username = $1
            )";

            pqxx::work txn(conn_);
            pqxx::result result = txn.exec_params(check_query, username);
            txn.commit();

            if (!result.empty()) { // пользователь найден               
                return true; 
            } else { // такого пользователя нет                
                return false;
            }

        } catch (const std::exception& e) {
            throw std::runtime_error("Ошибка при регистрации: " + std::string(e.what()));
        }
    };


    bool AuthService::VerifyPassword(const std::string& password, const std::string& stored_hash) {
        return password == stored_hash;
    };
} // namespace auth
