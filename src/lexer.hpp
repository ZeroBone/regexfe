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

    bool peek() {

        if (window.empty()) {
            assert(head == 0);
            // read from stream for the very first time
            if (char c; stream.get(c)) {
                window.push(c);
                return true;
            }
            return false;
        }

        assert(peek_offset < window.size());

        if (peek_offset < window.size() - 1) {
            // we want to peek into a position already present in the window, i.e., already obtained from the stream
            peek_offset += 1;
            return true;
        }

        // the peek position is currently pointing at the very end of the window
        // thus, we need to obtain a new character from the stream and increase the window
        if (char c; stream.get(c)) {
            window.push(c);
            peek_offset += 1;
            return true;
        }

        return false;
    }

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

    bool read() {
        if (window.empty()) {
            assert(head == 0);
            if (char c; stream.get(c)) {
                window.push(c);
                return true;
            }
            return false;
        }

        if (peek_offset >= 1) {
            assert(window.size() >= 2);
            window.pop();
            head++;
            peek_offset--;
            return true;
        }

        assert(peek_offset == 0);
        assert(!window.empty());

        if (window.size() >= 2) {
            window.pop();
            head++;
            return true;
        }

        assert(window.size() == 1);

        if (char c; stream.get(c)) {
            const std::string s(1, c);
            window = StringQueue(s);
            head++;
            return true;
        }

        return false;
    }
};

class LexerError final : public std::exception {

public:
    const size_t position;
    std::string message;

    explicit LexerError(const size_t position, std::string message) : position(position), message(std::move(message)) {}

    friend std::ostream& operator<<(std::ostream& stream, const LexerError& error) {
        return stream << "Syntax error at position " << (error.position + 1) << ": " << error.message;
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
