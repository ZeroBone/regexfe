#pragma once

#include <cassert>
#include <string>

class Token {

public:
    int id;
    size_t position;
    size_t length;

    explicit Token(const int id, const size_t position, const size_t length = 0) : id(id), position(position), length(length) {}

};