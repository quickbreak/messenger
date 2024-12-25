#include <pqxx/pqxx>
#include <string>
#include <iostream>


namespace auth {
    class AuthService {
    public:
        /// @brief конструктор
        /// @param connection_string строка подключения к БД
        explicit AuthService(const std::string& connection_string);
        /// @brief проверить наличие логина и верность пароля
        /// @param username логин
        /// @param password пароль
        /// @return вход разрешён?
        bool Authenticate(const std::string& username, const std::string& password);
        /// @brief проверить уникальность логина и сохранить нового пользователя
        /// @param username логин
        /// @param password пароль
        /// @return пользователь зарегистрирован?
        bool Register(const std::string& username, const std::string& password);
        /// @brief проверить, зарегистрирован ли пользователь username
        /// @param username username
        /// @return пользователь username зарегистрирован?
        bool Find(const std::string& username);
    private:
        /// @brief строка для соединения с бд
        std::string connection_string_;
        /// @brief проверить верность пароля
        /// @param password введённый пользователем пароль
        /// @param stored_hash сохранённый в бд пароль
        /// @return вход разрешён?
        bool static VerifyPassword(const std::string& password, const std::string& stored_hash);
    };
} // namespace auth
