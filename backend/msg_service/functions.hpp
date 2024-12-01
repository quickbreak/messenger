#pragma once


#include <iostream>


/// @brief функции, которые не связаны с классами, а значит их можно будет переиспользовать
namespace functions {
    /// @brief подготовить ответ
    /// @param message полученное сообщение
    /// @return ответное сообщение
    std::string GenerateResponse(std::string message);
    /// @brief обработать запрос
    /// @param request запрос, json в виде строки
    /// @return ответ, json в виде строки
    std::string HandleRequest(std::string request);
} // namespace functions
