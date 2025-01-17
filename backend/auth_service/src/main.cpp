#include <crow.h>
#include <crow/middlewares/cors.h>

#include <iostream>
#include <cstdlib>

#include "./auth.hpp"


const char* db_name = std::getenv("DB_NAME");
const char* db_user = std::getenv("DB_USER");
const char* db_password = std::getenv("DB_PASSWORD");
const char* db_host = std::getenv("DB_HOST");
const char* db_port = std::getenv("DB_PORT");
const std::string connection_string = "dbname=" + std::string(db_name) + " user=" + std::string(db_user) + " password=" + std::string(db_password) + " host=" + std::string(db_host) + " port=" + std::string(db_port);


bool CheckAuth(const std::string& username, const std::string& password) {
    // std::cout << "Ошибка после входа в Check\n";
    auth::AuthService connector = auth::AuthService(connection_string);
    return connector.Authenticate(username, password);
}

bool TryToReg(const std::string& username, const std::string& password) {
    // std::cout << "Ошибка после входа в TryToReg\n";
    auth::AuthService connector = auth::AuthService(connection_string);
    return connector.Register(username, password);
}

bool Find(const std::string& username) {
    auth::AuthService connector = auth::AuthService(connection_string);
    return connector.Find(username);
}


int main() {
    //crow::SimpleApp app;
    crow::App<crow::CORSHandler> app;


    // Обработка GET-запроса на эндпоинт /
    CROW_ROUTE(app, "/")
    .methods(crow::HTTPMethod::GET)([] (const crow::request& req) {
        std::cout << "Received GET request on /" << std::endl;

        crow::response res;
        // подготовить ответ
        crow::json::wvalue response;
        response["response"] = "Hello from auth_service running on :5000";
        res.code = 200;
        res.body = response.dump();
        std::cout << "Replied with '{\"response\":\"Hello from auth_service running on :5000\"}'" << std::endl;
        return res;
    });


    // Обработка POST-запроса на эндпоинт /auth
    CROW_ROUTE(app, "/auth")
    .methods(crow::HTTPMethod::POST)([] (const crow::request& req) {
        std::cout << "Received POST request on /auth" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        crow::response res;
        try {
            // тело запроса как JSON
            auto body = crow::json::load(req.body);

            // некорректный запрос
            if (!body || !body.has("username") || !body.has("password")) {
                res.body = "Invalid JSON format. Expected 'username'.";
                res.code = 400;
                return res;
            }

            // извлечь данные из запроса
            std::string username = body["username"].s();
            std::string password = body["password"].s();
            // подготовить ответ
            crow::json::wvalue response;
            if (CheckAuth(username, password)) {
                response["status"] = "success";
                response["comment"] = "Welcome!";
                res.code = 200;
            } else {
                response["status"] = "failure";
                response["comment"] = "Wrong username or/and password.";
                res.code = 401;
            }
            res.body = response.dump();
            return res;
        } catch (const std::exception& e) {
            res.body = "Server error: " + std::string(e.what());
            res.code = 500;
            return res;
        }
    });


    // Обработка POST-запроса на эндпоинт /reg
    CROW_ROUTE(app, "/reg")
    .methods(crow::HTTPMethod::POST)([] (const crow::request& req) {
        std::cout << "Received POST request on /reg" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        crow::response res;
        try {
            // тело запроса как JSON
            auto body = crow::json::load(req.body);

            // некорректный запрос
            if (!body || !body.has("username") || !body.has("password")) {
                res.body = "Invalid JSON format. Expected 'username'.";
                res.code = 400;
                return res;
            }

            // извлечь данные из запроса
            std::string username = body["username"].s();
            std::string password = body["password"].s();
            // подготовить ответ
            crow::json::wvalue response;
            if (TryToReg(username, password)) {
                response["status"] = "success";
                response["comment"] = "Welcome!";
                res.code = 200;
            } else {
                response["status"] = "failure";
                response["comment"] = "We`re sorry, but this username is already taken.";
                res.code = 409;
            }
            res.body = response.dump();
            return res;
        } catch (const std::exception& e) {
            res.body = "Server error: " + std::string(e.what());
            res.code = 500;
            return res;
        }
    });


    // Обработка GET-запроса на эндпоинт /find
    CROW_ROUTE(app, "/find")
    .methods(crow::HTTPMethod::GET)([] (const crow::request& req) {
        std::cout << "Received GET request on /find" << std::endl;

        crow::response res;
        try {
            // извлечь данные из запроса
            auto username_param = req.url_params.get("username");

            // некорректный запрос
            if (!username_param) {
                res.body = "Invalid request format. Expected '?username=username'.";
                res.code = 400;
                return res;
            }

            std::string username = std::string(username_param);
            // подготовить ответ
            crow::json::wvalue response;
            if (Find(username)) {
                // response["status"] = "success";
                response["comment"] = "Приятного общения :)";
                res.code = 200;
            } else {
                // response["status"] = "failure";
                response["comment"] = "Пользователь с таким именем не зарегистрирован";
                res.code = 409;
            }
            res.body = response.dump();
            return res;
        } catch (const std::exception& e) {
            res.body = "Server error: " + std::string(e.what());
            res.code = 500;
            return res;
        }
    });


    // std::cout << "Сервер будет запущен на порту 5000.\n";
    //crow::logger::setLogLevel(crow::LogLevel::Debug);

    // Запуск сервера на порту 5000
    app.port(5000).multithreaded().run();

    return 0;
}
