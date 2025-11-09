#pragma once

#include <exception>
#include <string>

#include "ast.hpp"

class ParserError final : public std::exception {

public:
    const size_t position;
    std::string message;

    explicit ParserError(const size_t position, std::string message) : position(position), message(std::move(message)) {}

    friend std::ostream& operator<<(std::ostream& stream, const ParserError& error) {
        return stream << (error.position + 1) << ": error: parsing error: " << error.message;
    }

    [[nodiscard]] const char* what() const noexcept override {
        return "Parser error.";
    }
};

Expression* parse_regex(const std::string& regex);