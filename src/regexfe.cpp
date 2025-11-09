#include "regexfe.hpp"

#include "Parser.h"
#include "lexer.hpp"

Expression* parse_regex(const std::string& regex) {

    Lexer lexer(regex);
    Parser parser;
    size_t last_token_position = 0;

    while (true) {

        Token token = lexer.lex();

        last_token_position = token.position;

        Parser::StackEntryPayload payload;

        switch (token.id) {
            case T_CHARACTER:
            case T_SPECIAL_CHARACTER:
                payload.CHARACTER = token.payload.front();
                break;

            default:
                break;
        }

        if (!parser.parse(token.id, payload)) {
            break;
        }

        if (token.id == T_EOF) {
            break;
        }

    }

    if (!parser.successfullyParsed()) {
        throw ParserError(last_token_position, "invalid syntax.");
    }

    return parser.getValue().expression;

}