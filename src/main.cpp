#include <fstream>
#include <iostream>

#include "Parser.h"
#include "lexer.hpp"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file_name> <regex_pattern> [--dump-mim]" << std::endl;
        return 0xdead;
    }

    std::string file_name = argv[1];
    std::string regex_pattern = argv[2];

    bool dump_mim = false;
    if (argc == 4) {
        if (std::string flag = argv[3]; flag == "--dump-mim") {
            dump_mim = true;
        }
        else {
            std::cerr << "Unknown option: " << flag << "\n";
            return 0xbeef;
        }
    }

    std::ifstream input_file(file_name);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open file '" << file_name << "' for reading." << std::endl;
        return 0xcafe;
    }

    // parse regular expression

    /*if (regex_pattern.empty()) {
        std::cerr << "Syntax error: regex is empty." << std::endl;
        return 0xafd;
    }*/

    Lexer lexer(regex_pattern);
    Parser parser;

    while (true) {

        ZBResult<Token, LexerError> result = lexer.lex();

        if (result.is_error()) {
            LexerError error = result.unwrap_error();

            std::cerr << "Syntax error at position " << error.position << ": " << error.message << std::endl;
            return 0xaffe;
        }

        Token token = result.unwrap();
        parser.parse(token.id, Parser::StackEntryPayload{});

        if (token.id == T_EOF) {
            break;
        }

    }

    if (!parser.successfullyParsed()) {
        std::cerr << "Error: regular expression is invalid." << std::endl;
    }

    std::cout << "Regex pattern: " << regex_pattern << "\n";
    std::cout << "Dump MIM: " << (dump_mim ? "true" : "false") << "\n";

    input_file.close();

    return 0;
}
