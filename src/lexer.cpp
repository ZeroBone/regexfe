#include "lexer.hpp"

#include "Parser.h"

bool LexerBackend::peek() {

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

bool LexerBackend::read() {
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

Token Lexer::lex() {

    if (!backend.read()) {
        return Token(T_EOF, backend.head_position());
    }

    backend.move_peek_to_head();
    assert(backend.peek_position() == backend.head_position());

    // ReSharper disable once CppTooWideScope
    const char head_char = backend.char_at_head();

    switch (head_char) {

        case '(': {

            // maximal munch strategy
            if (backend.peek() && backend.char_at_peek() == '?' && backend.peek() && backend.char_at_peek() == ':') {
                backend.move_head_to_peek();
                return Token(T_LEFT_PARENTHESIS_QUESTION_MARK_COLON, backend.head_position());
            }

            return Token(T_LEFT_PARENTHESIS, backend.head_position());
        }

        case '|':
            return Token(T_OR, backend.head_position());

        case ')':
            return Token(T_RIGHT_PARENTHESIS, backend.head_position());

        case '*':
            return Token(T_STAR, backend.head_position());

        case '+':
            return Token(T_PLUS, backend.head_position());

        case '-':
            return Token(T_MINUS, backend.head_position());

        case '?':
            return Token(T_QUESTION_MARK, backend.head_position());

        case '.':
            return Token(T_DOT, backend.head_position());

        case '[':
            return Token(T_LEFT_BRACKET, backend.head_position());

        case ']':
            return Token(T_RIGHT_BRACKET, backend.head_position());

        case '^':
            return Token(T_UP_ARROW, backend.head_position());

        case '\\': {
            if (!backend.peek()) {
                throw LexerError(backend.head_position(), "Invalid start of lexeme: '\\'.");
            }
            // ReSharper disable once CppTooWideScope
            const char peeked_char = backend.char_at_peek();
            switch (peeked_char) {
                case 'w': {
                    backend.move_head_to_peek();
                    return Token(T_WORD_CHARS, backend.head_position());
                }
                case 'W': {
                    backend.move_head_to_peek();
                    return Token(T_NON_WORD_CHARS, backend.head_position());
                }
                case 'd': {
                    backend.move_head_to_peek();
                    return Token(T_DIGIT_CHARS, backend.head_position());
                }
                case 'D': {
                    backend.move_head_to_peek();
                    return Token(T_NON_DIGIT_CHARS, backend.head_position());
                }
                case 's': {
                    backend.move_head_to_peek();
                    return Token(T_WHITESPACE_CHARS, backend.head_position());
                }
                case 'S': {
                    backend.move_head_to_peek();
                    return Token(T_NON_WHITESPACE_CHARS, backend.head_position());
                }
                case 't': {
                    backend.move_head_to_peek();
                    return Token(T_SPECIAL_CHARACTER, backend.head_position(), "\t");
                }
                case 'n': {
                    backend.move_head_to_peek();
                    return Token(T_SPECIAL_CHARACTER, backend.head_position(), "\n");
                }
                case 'r': {
                    backend.move_head_to_peek();
                    return Token(T_SPECIAL_CHARACTER, backend.head_position(), "\r");
                }
                case '.':
                case '*':
                case '+':
                case '?':
                case '(':
                case ')':
                case '[':
                case ']':
                case '|':
                case '^':
                case '\\': {
                    backend.move_head_to_peek();
                    const std::string payload(1, peeked_char);
                    return Token(T_SPECIAL_CHARACTER, backend.head_position(), payload);
                }
                
                default: {
                    std::stringstream ss;
                    ss << "Invalid escape sequence: '\\' cannot be followed by '" << peeked_char << "'.";
                    throw LexerError(backend.head_position(), ss.str());
                }
            }
        }

        default: {

            if (std::isprint(static_cast<unsigned char>(head_char))) {
                std::string payload(1, head_char);
                return Token(T_CHARACTER, backend.head_position(), payload);
            }

            std::stringstream ss;
            ss << "Unexpected character '" << head_char << "'.";
            throw LexerError(backend.head_position(), ss.str());
        }

    }
}