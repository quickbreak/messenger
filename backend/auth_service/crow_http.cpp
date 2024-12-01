#include <crow.h>
#include <crow/middlewares/cors.h>

#include <iostream>

#include "./auth.cpp"


bool CheckAuth(const std::string& username, const std::string& password) {
    // std::cout << "Ошибка после входа в Check\n";
    auth::AuthService connector = auth::AuthService("dbname=mydb user=postgres password=mypassword host=localhost port=5432");
    return connector.Authenticate(username, password);
}


int main() {
    //crow::SimpleApp app;
    crow::App<crow::CORSHandler> app;


    // Обработка POST-запроса на эндпоинт /auth
    CROW_ROUTE(app, "/auth")
    .methods(crow::HTTPMethod::POST)([] (const crow::request& req) {
        //
        std::cout << "Received POST request on /auth" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        crow::response res;
        try {
            // тело запроса как JSON
            auto body = crow::json::load(req.body);

            // некорректный запрос
            if (!body || !body.has("username")) {
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

    // std::cout << "Поехали!\n";
    //crow::logger::setLogLevel(crow::LogLevel::Debug);

    // Запуск сервера на порту 5000
    app.port(5000).multithreaded().run();

    return 0;
}
