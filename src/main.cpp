#include <fstream>
#include <iostream>

#include "Parser.h"
#include "lexer.hpp"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file_name> <regex_pattern> [--dump-mim]" << std::endl;
        return 1;
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
            return 2;
        }
    }

    std::ifstream input_file(file_name);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open file '" << file_name << "' for reading." << std::endl;
        return 3;
    }

    // parse regular expression

    Lexer lexer(regex_pattern);
    Parser parser;

    while (true) {

        Token token;

        try {
            token = lexer.lex();
        }
        catch (const LexerError& e) {
            std::cerr << e << std::endl;
            return 4;
        }

        std::cout << token.id << " at " << token.position << std::endl;

        if (!parser.parse(token.id, Parser::StackEntryPayload{})) {
            break;
        }

        if (token.id == T_EOF) {
            break;
        }

    }

    if (!parser.successfullyParsed()) {
        std::cerr << "Syntax error: regular expression is invalid." << std::endl;
        return 5;
    }

    std::cout << "Success." << std::endl;

    // std::cout << "Regex pattern: " << regex_pattern << "\n";
    // std::cout << "Dump MIM: " << (dump_mim ? "true" : "false") << "\n";

    input_file.close();

    return 0;
}
