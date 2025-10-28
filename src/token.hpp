#pragma once

#include <cassert>
#include <string>

class Token {

public:
    int id = 0;
    size_t position = 0;

    explicit Token(const int id, const size_t position) : id(id), position(position) {}

    Token() = default;

};