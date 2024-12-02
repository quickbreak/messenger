#include <crow.h>
#include <crow/middlewares/cors.h>

#include <iostream>

#include "./auth.hpp"


bool CheckAuth(const std::string& username, const std::string& password) {
    // std::cout << "Ошибка после входа в Check\n";
    auth::AuthService connector = auth::AuthService("dbname=mydb user=postgres password=mypassword host=localhost port=5432");
    return connector.Authenticate(username, password);
}

bool TryToReg(const std::string& username, const std::string& password) {
    // std::cout << "Ошибка после входа в TryToReg\n";
    auth::AuthService connector = auth::AuthService("dbname=mydb user=postgres password=mypassword host=localhost port=5432");
    return connector.Register(username, password);
}

bool Find(const std::string& username) {
    auth::AuthService connector = auth::AuthService("dbname=mydb user=postgres password=mypassword host=localhost port=5432");
    return connector.Find(username);
}


int main() {
    //crow::SimpleApp app;
    crow::App<crow::CORSHandler> app;


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


    // Обработка GET-запроса на эндпоинт /reg
    CROW_ROUTE(app, "/find")
    .methods(crow::HTTPMethod::GET)([] (const crow::request& req) {
        std::cout << "Received GET request on /reg" << std::endl;

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


    // std::cout << "Поехали!\n";
    //crow::logger::setLogLevel(crow::LogLevel::Debug);

    // Запуск сервера на порту 5000
    app.port(5000).multithreaded().run();

    return 0;
}
