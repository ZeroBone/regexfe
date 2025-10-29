#pragma once

#include <cassert>

#include <sstream>
#include <string>
#include <utility>

#include "strqueue.hpp"
#include "token.hpp"

class LexerBackend {
    std::istringstream stream;

    size_t head = 0;
    size_t peek_offset = 0;

    // invariant: window is the string between head and the last character of the stream that was read so far
    // invariant: either head = 0 and window = "" or 0 <= peek_offset < window.size()
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

    [[nodiscard]] size_t head_position() const {
        return head;
    }

    [[nodiscard]] size_t peek_position() const {
        return head + peek_offset;
    }

    bool peek();
    bool read();

    void move_peek_to_head() {
        peek_offset = 0;
    }

    std::string string_between_head_and_peek() const {
        assert(!window.empty());
        assert(peek_offset < window.size());
        return std::string(window.view().substr(0, peek_offset));
    }

    void move_head_to_peek() {
        head += peek_offset;
        window.pop(peek_offset);
        peek_offset = 0;
    }

};

class LexerError final : public std::exception {

public:
    const size_t position;
    std::string message;

    explicit LexerError(const size_t position, std::string message) : position(position), message(std::move(message)) {}

    friend std::ostream& operator<<(std::ostream& stream, const LexerError& error) {
        return stream << (error.position + 1) << ": error: lexing error: " << error.message;
    }

    [[nodiscard]] const char* what() const noexcept override {
        return "Lexer error.";
    }
};

class Lexer {

    LexerBackend backend;

public:
    explicit Lexer(const std::string& input) : backend(LexerBackend(input)) {}

    Token lex();

};
