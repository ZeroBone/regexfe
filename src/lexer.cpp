#include "lexer.hpp"

#include "Parser.h"


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
                default: {
                    std::stringstream ss;
                    ss << "Invalid escape sequence: '\\' cannot be followed by '" << peeked_char << "'.";
                    throw LexerError(backend.head_position(), ss.str());
                }
            }
        }

        default: {
            std::stringstream ss;
            ss << "Unexpected character '" << head_char << "'.";
            throw LexerError(backend.head_position(), ss.str());
        }

    }
}