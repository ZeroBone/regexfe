#include <fstream>
#include <iostream>

#include "Parser.h"
#include "lexer.hpp"
#include "ast.hpp"
#include "mimir.hpp"
#include "mimir_codegen.hpp"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <regex_pattern> <file_name> [--dump-mim]" << std::endl;
        return 2;
    }

    std::string regex_pattern = argv[1];
    std::string file_name = argv[2];

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

    // parse regular expression

    Lexer lexer(regex_pattern);
    Parser parser;
    size_t last_token_position = 0;

    while (true) {

        Token token;

        try {
            token = lexer.lex();
        }
        catch (const LexerError& e) {
            std::cerr << e << std::endl;
            return 1;
        }

        // std::cout << token.id << std::endl;

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
        std::cerr << (last_token_position + 1) << ": error: parsing error: invalid syntax." << std::endl;
        return 1;
    }

    Expression* expression = parser.getValue().expression;
    MimirCodeGen code_gen;

    MimRegex regex = expression->generateMimIR(code_gen);

    if (dump_mim) {
        std::cout << regex << std::endl;
        return 0;
    }

    std::ifstream input_file(file_name);
    if (!input_file.is_open()) {
        std::cerr << "0: error: could not open file '" << file_name << "' for reading." << std::endl;
        return 2;
    }

    std::function<bool(const char*)> matcher = code_gen.make_matcher(regex);

    std::string line;
    while (std::getline(input_file, line)) {
        bool matched = matcher(line.c_str());
        std::cout << line << "," << (matched ? "true" : "false") << std::endl;
    }

    input_file.close();

    return 0;
}
