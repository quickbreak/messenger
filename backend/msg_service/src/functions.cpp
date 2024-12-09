#include "functions.hpp"

#include <boost/json.hpp>


std::string functions::GenerateResponse(std::string message)
{
    auto pos = message.find("PING");
    if (pos != std::string::npos && pos == 0) {
        if (message.size() == std::string("PING").size()) { // полагаю, это быстрее, чем сравнивать строки
            // 'PING' -> 'PONG'
            message = "PONG";
        } else {
            // 'PING hello' -> 'hello'
            message = message.substr(pos + std::string("PING").size() + 1);
        }
    }
    else if ((pos = message.find("ping")) != std::string::npos && pos == 0) {
        if (message.size() == std::string("ping").size()) {
            // 'ping' -> 'pong'
            message = "pong";
        } else {
            // 'ping hello' -> 'hello'
            message = message.substr(pos + std::string("ping").size() + 1);
        }
    }
    else {
        // 'hello' -> 'hello'
    }

    return std::move(message);
}

namespace json = boost::json;

std::string functions::HandleRequest(std::string request)
{
    json::value parsed_json = json::parse(request);
    json::object obj = std::move(parsed_json.as_object());
    std::string received_message = obj["message"].as_string().c_str();
    std::string response_message = GenerateResponse(std::move(received_message));
    obj["message"] = response_message;
    return std::move(json::serialize(obj));
}
