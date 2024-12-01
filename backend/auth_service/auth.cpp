#include <pqxx/pqxx>
#include <string>
#include <stdexcept>
#include <iostream>


namespace auth {
  class AuthService {
  public:
    // Конструктор, принимает строку подключения к БД
    explicit AuthService(const std::string& conn_str) 
      : conn(conn_str) {}

    // Метод для проверки логина и пароля
    bool Authenticate(const std::string& username, const std::string& password) {
        try {
            pqxx::work txn(conn);
            // std::cout << "Ошибка после подключения\n";

            const std::string query = R"(
            SELECT password 
            FROM users
            WHERE username = $1
            )";

            pqxx::result result = txn.exec_params(query, username);

            if (result.empty()) {
            // Пользователь не найден
                return false;
            }

            std::string stored_hash = result[0]["password"].as<std::string>();

            // Проверяем введённый пароль
            return VerifyPassword(password, stored_hash);

        } catch (const std::exception& e) {
            throw std::runtime_error("Ошибка при аутентификации: " + std::string(e.what()));
        }
    }


  private:
    pqxx::connection conn;

    // Пример функции для проверки пароля (добавьте свою логику хэширования)
    bool VerifyPassword(const std::string& password, const std::string& stored_hash) {
      // Для упрощения, сравниваем напрямую (НЕ ДЕЛАЙТЕ ТАК В РЕАЛЬНЫХ ПРОЕКТАХ!)
      return password == stored_hash;
    }
  };
} // namespace auth
