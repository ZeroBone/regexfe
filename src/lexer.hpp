#pragma once

#include <cassert>

#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include "strqueue.hpp"
#include "token.hpp"

class LexerBackend {
    std::istringstream stream;

    size_t head_pos = std::numeric_limits<size_t>::max();

    // invariant: window is the string between head_position and peek_position
    StringQueue window = StringQueue("");

public:
    explicit LexerBackend(const std::string& source) : stream(source) {}

    [[nodiscard]] char char_at_head() const {
        assert(!window.empty());
        return window.front();
    }

    [[nodiscard]] char char_at_peek() const {
        assert(!window.empty());
        return window.back();
    }

    [[nodiscard]] bool window_empty() const {
        return window.empty();
    }

    [[nodiscard]] size_t head_position() const {
        if (head_pos == std::numeric_limits<size_t>::max()) {
            return 0;
        }
        return head_pos;
    }

    [[nodiscard]] size_t peeking_position() const {

        if (head_pos != std::numeric_limits<size_t>::max()) {
            return head_pos + window.size();
        }

        assert(!window.empty());
        return window.size() - 1;

    }

    bool peek() {
        if (char c; stream.get(c)) {
            window.push(c);
            return true;
        }

        return false;
    }

    bool read() {

        if (window.empty()) {
            char c;
            return static_cast<bool>(stream.get(c));
        }

        window.pop();

        if (head_pos == std::numeric_limits<size_t>::max()) {
            head_pos = 0;
        }
        else {
            head_pos++;
        }

        return true;
    }

};

class LexerError : public std::exception {

public:
    const size_t position;
    std::string message;

    explicit LexerError(const size_t position, std::string message) : position(position), message(std::move(message)) {}

    [[nodiscard]] bool is_eof_error() const {
        return message.empty();
    }

    static LexerError reachedEof(const size_t position) {
        return LexerError(position, "");
    }

};

class Lexer {

    LexerBackend backend;

public:
    explicit Lexer(const std::string& input) : backend(LexerBackend(input)) {}

    Token lex();

};
