#include <fstream>
#include <iostream>

#include "ast.hpp"
#include "lexer.hpp"
#include "mimir.hpp"
#include "mimir_codegen.hpp"
#include "regexfe.hpp"
#include "tests.hpp"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        if (argc == 2) {
            if (std::string first_arg = argv[1]; first_arg == "--run-tests") {
                return run_tests();
            }
        }
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
    Expression* expression;
    try {
        expression = parse_regex(regex_pattern);
    }
    catch (const LexerError& e) {
        std::cerr << e << std::endl;
        return 1;
    }
    catch (const ParserError& e) {
        std::cerr << e << std::endl;
        return 1;
    }

    MimirCodeGen code_gen;
    MimRegex regex = expression->generateMimIR(code_gen);

    delete expression;

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
