#include "lexer.hpp"
#include "Parser.h"


Token Lexer::lex() {

    backend.move_peek_to_head();

    if (!backend.peek()) {
        return Token(T_EOF, backend.head_position());
    }

    switch (backend.char_at_head()) {

        case '(': {
            bool read_qmark = backend.peek() && backend.char_at_peek() == '?';
            bool read_colon = read_qmark && backend.peek() && backend.char_at_peek() == ':';

            if (read_qmark && read_colon) {

            }
        }

    }
}