#include "functions.hpp"


std::string functions::handle_message(std::string message)
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