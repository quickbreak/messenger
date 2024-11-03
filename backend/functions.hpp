#pragma once


#include <iostream>

/// @brief функции, которые не связаны с классами, а значит их можно будет переиспользовать
namespace functions {
    /// @brief подготовить ответ
    /// @param message полученное сообщение
    /// @return ответное сообщение
    std::string handle_message(std::string message);
} // namespace functions