#include "auth.hpp"


namespace auth {
    AuthService::AuthService(const std::string& connection_string) : 
        connection_string_(connection_string), 
        db_connection_(nullptr)  
    {}


    void AuthService::EnsureConnection() {
        // если бд была перезапущена, соединение было разорвано
        // тогда его нужно установить заново. Семён Семёныч!!
        if (!this->db_connection_ || !this->db_connection_->is_open()) {
            this->db_connection_ = std::make_unique<pqxx::connection>(this->connection_string_);
            std::cout << "Reconnected to the database.\n";
        }
    }


    bool AuthService::Authenticate(const std::string& username, const std::string& password) {
        const std::string query = R"(
            SELECT password 
            FROM users
            WHERE username = $1
            )";
        try {
            EnsureConnection();    
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(query, username);
            transaction.commit();

            if (result.empty()) { // Пользователь не найден
                return false;
            }

            std::string stored_hash = result[0]["password"].as<std::string>();

            return VerifyPassword(password, stored_hash);
        } catch (const std::exception& e) {
            std::cerr << "AuthService::Authenticate error: " << e.what() << '\n';
            throw;
            // throw std::runtime_error("Ошибка при регистрации: " + std::string(e.what()));
        }
    };


    bool AuthService::Register(const std::string& username, const std::string& password) {
        const std::string check_query = R"(
        SELECT username 
        FROM users
        WHERE username = $1
        )";
        try {
            EnsureConnection();    
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(check_query, username);
            transaction.commit();

            if (result.empty()) { // логин не занят, регистрируем нового пользователя
                const std::string register_query = R"(
                INSERT INTO users 
                (username, password)
                VALUES ($1, $2)
                )";

                EnsureConnection();    
                pqxx::work transaction(*db_connection_);
                pqxx::result result = transaction.exec_params(register_query, username, password);
                transaction.commit();
                
                return true; 
            } else { // логин занят
                return false;
            }

        } catch (const std::exception& e) {
            std::cerr << "AuthService::Authenticate error: " << e.what() << '\n';
            throw;
            // throw std::runtime_error("Ошибка при регистрации: " + std::string(e.what()));
        }
    };


    bool AuthService::Find(const std::string& username) {
        const std::string check_query = R"(
        SELECT username 
        FROM users
        WHERE username = $1
        )";
        try {
            EnsureConnection();    
            pqxx::work transaction(*db_connection_);
            pqxx::result result = transaction.exec_params(check_query, username);
            transaction.commit();

            if (!result.empty()) { // пользователь найден               
                return true; 
            } else { // такого пользователя нет                
                return false;
            }

        } catch (const std::exception& e) {
            std::cerr << "AuthService::Authenticate error: " << e.what() << '\n';
            throw;
            // throw std::runtime_error("Ошибка при регистрации: " + std::string(e.what()));
        }
    };


    bool AuthService::VerifyPassword(const std::string& password, const std::string& stored_hash) {
        return password == stored_hash;
    };
} // namespace auth
