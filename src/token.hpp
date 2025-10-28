#pragma once

#include <string>

class Token {

public:
    int id = 0;
    size_t position = 0;
    std::string payload;

    explicit Token(const int id, const size_t position, std::string payload = "") : id(id), position(position), payload(std::move(payload)) {}

    Token() = default;

};