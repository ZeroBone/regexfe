/*
 * Author: Bashar Hamade
 * (Minor modifications by Alexander Mayorov)
 * This file is not covered by the LICENSE
 **/

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"
#include "regexfe.hpp"

struct TestCase {
    std::string regex;
    bool should_fail;
    std::string description;
};

struct TestResult {
    int passed = 0;
    int total = 0;
    std::vector<std::string> failures;
};

// Always show MimIR for successful parses
void test_regex(const std::string& regex, bool should_fail, const std::string& description, TestResult& result) {
    std::cout << "\n  ┌─ Test: " << description << "\n";
    std::cout << "  │ Regex: \"" << regex << "\"\n";
    std::cout << "  │ Expect: " << (should_fail ? "FAIL" : "PASS") << "\n";

    result.total++;

    Expression* expression;

    try {
        expression = parse_regex(regex);
    }
    catch (const LexerError& e) {
        if (should_fail) {
            std::cout << "  │ Result: ✅ EXPECTED ERROR\n";
            std::cout << "  │ Error: " << e << "\n";
            std::cout << "  └─ Status: PASS\n";
            result.passed++;
        }
        else {
            std::cout << "  │ Result: ❌ UNEXPECTED ERROR\n";
            std::cout << "  │ Error: " << e << "\n";
            std::cout << "  └─ Status: FAIL\n";
            result.failures.push_back(description + ": \"" + regex + "\" (unexpected error)");
        }
        return;
    }
    catch (const ParserError& e) {
        if (should_fail) {
            std::cout << "  │ Result: ✅ EXPECTED ERROR\n";
            std::cout << "  │ Error: " << e << "\n";
            std::cout << "  └─ Status: PASS\n";
            result.passed++;
        }
        else {
            std::cout << "  │ Result: ❌ UNEXPECTED ERROR\n";
            std::cout << "  │ Error: " << e << "\n";
            std::cout << "  └─ Status: FAIL\n";
            result.failures.push_back(description + ": \"" + regex + "\" (unexpected error)");
        }
        return;
    }
    catch (...) {
        std::cout << "  │ Result: ❌ UNKNOWN EXCEPTION\n";
        std::cout << "  └─ Status: FAIL\n";
        result.failures.push_back(description + ": \"" + regex + "\" (unknown exception)");
        return;
    }

    if (should_fail) {
        std::cout << "  │ Result: ❌ UNEXPECTED SUCCESS\n";
        std::cout << "  └─ Status: FAIL\n";
        result.failures.push_back(description + ": \"" + regex + "\" (should have failed)");
        return;
    }

    MimirCodeGen code_gen;
    MimRegex mim_result = expression->generateMimIR(code_gen);

    // Show MimIR output
    std::ostringstream oss;
    oss << mim_result;
    std::string mimir_str = oss.str();

    // Clean up newlines
    std::erase(mimir_str, '\n');

    std::cout << "  │ Result: ✅ SUCCESS\n";
    std::cout << "  │ MimIR: " << mimir_str << "\n";
    std::cout << "  └─ Status: PASS\n";
    result.passed++;

}

void run_test_section(const std::string& section_name, const std::vector<TestCase>& tests, TestResult& result) {
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║ " << std::left << std::setw(62) << section_name << "║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n";

    for (const auto& test : tests) {
        test_regex(test.regex, test.should_fail, test.description, result);
    }
}

int run_tests() {
    TestResult result;

    std::cout << "\n";
    std::cout << "████████████████████████████████████████████████████████████████\n";
    std::cout << "██                                                            ██\n";
    std::cout << "██       COMPREHENSIVE REGEX PARSER TEST SUITE                ██\n";
    std::cout << "██       (Based on Server Test Cases)                         ██\n";
    std::cout << "██                                                            ██\n";
    std::cout << "████████████████████████████████████████████████████████████████\n";

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/empty
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/empty - Empty Regex (ε)", {
        {"", false, "Empty regex matches empty string"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/lit-pass-fail
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/lit-pass-fail - Literal Characters", {
        {"a", false, "Single lowercase letter"},
        {"Z", false, "Single uppercase letter"},
        {"5", false, "Single digit"},
        {" ", false, "Single space"},
        {"!", false, "Single punctuation"},
        {"abc", false, "Multiple literals (concatenation)"},
        {"hello", false, "Word literal"},
        {"123", false, "Number literal"},
        {"a b c", false, "Literals with spaces"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/any
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/any - Dot (Any Character)", {
        {".", false, "Single dot"},
        {"..", false, "Two dots"},
        {"...", false, "Three dots"},
        {"a.b", false, "Dot in middle"},
        {".a", false, "Dot at start"},
        {"a.", false, "Dot at end"},
        {"a.b.c", false, "Multiple dots in sequence"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/special_chars
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/special_chars - Escaped Special Characters", {
        {"\\*", false, "Escaped asterisk"},
        {"\\+", false, "Escaped plus"},
        {"\\?", false, "Escaped question"},
        {"\\(", false, "Escaped open paren"},
        {"\\)", false, "Escaped close paren"},
        {"\\[", false, "Escaped open bracket"},
        {"\\]", false, "Escaped close bracket"},
        {"\\|", false, "Escaped pipe"},
        {"\\\\", false, "Escaped backslash"},
        {"\\.", false, "Escaped dot"},
        {"\\t", false, "Tab escape"},
        {"a\\*b", false, "Escaped asterisk in middle"},
        {"\\(a\\)", false, "Escaped parens around literal"},
        {"\\[a\\]", false, "Escaped brackets around literal"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/wds_star
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/wds_star - Character Classes with Star", {
        {"\\w*", false, "Word chars, zero or more"},
        {"\\d*", false, "Digits, zero or more"},
        {"\\s*", false, "Whitespace, zero or more"},
        {"a\\w*", false, "'a' followed by word chars"},
        {"\\w*b", false, "Word chars followed by 'b'"},
        {"\\d*\\w*", false, "Digits then word chars"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/wds_plus
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/wds_plus - Character Classes with Plus", {
        {"\\w+", false, "Word chars, one or more"},
        {"\\d+", false, "Digits, one or more"},
        {"\\s+", false, "Whitespace, one or more"},
        {"\\w+\\d+", false, "Word chars then digits"},
        {"a\\w+b", false, "'a', word chars, 'b'"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/wds_question
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/wds_question - Character Classes with Question", {
        {"\\w?", false, "Optional word char"},
        {"\\d?", false, "Optional digit"},
        {"\\s?", false, "Optional whitespace"},
        {"a\\w?b", false, "'a', optional word char, 'b'"},
        {"\\d?\\w?", false, "Optional digit, optional word char"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/WDS
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/WDS - Negated Character Classes", {
        {"\\W", false, "Non-word character"},
        {"\\D", false, "Non-digit"},
        {"\\S", false, "Non-whitespace"},
        {"\\W+", false, "One or more non-word"},
        {"\\D*", false, "Zero or more non-digit"},
        {"\\S?", false, "Optional non-whitespace"},
        {"\\w\\W", false, "Word char then non-word char"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/wors
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/wors - Word or Space Patterns", {
        {"\\w|\\s", false, "Word char OR space"},
        {"[\\w\\s]", false, "Character set: word or space"},
        {"[\\w\\s]+", false, "One or more word or space"},
        {"\\w+|\\s+", false, "Word chars OR spaces"},
        {"(\\w|\\s)*", false, "Zero or more word or space"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/char_range
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/char_range - Single Character Range", {
        {"[a-z]", false, "Lowercase range"},
        {"[A-Z]", false, "Uppercase range"},
        {"[0-9]", false, "Digit range"},
        {"[a-f]", false, "Hex lowercase range"},
        {"[A-F]", false, "Hex uppercase range"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/char_ranges
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/char_ranges - Multiple Character Ranges", {
        {"[a-zA-Z]", false, "Upper and lowercase"},
        {"[a-z0-9]", false, "Lowercase and digits"},
        {"[A-Z0-9]", false, "Uppercase and digits"},
        {"[a-zA-Z0-9]", false, "Alphanumeric"},
        {"[a-z0-9_]", false, "Alphanumeric plus underscore"},
        {"[a-zA-Z0-9_]", false, "Identifier pattern"},
        {"[a-fA-F0-9]", false, "Hex digits"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/bracket_range
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/bracket_range - Character Sets (Brackets)", {
        {"[a]", false, "Single char in set"},
        {"[abc]", false, "Multiple chars in set"},
        {"[aeiou]", false, "Vowels"},
        {"[a-z]", false, "Range in brackets"},
        {"[abcxyz]", false, "Individual chars"},
        {"[a-cx-z]", false, "Multiple ranges"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/neg_char_range
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/neg_char_range - Negated Single Range", {
        {"[^a-z]", false, "NOT lowercase"},
        {"[^A-Z]", false, "NOT uppercase"},
        {"[^0-9]", false, "NOT digit"},
        {"[^a]", false, "NOT 'a'"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/neg_char_ranges
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/neg_char_ranges - Negated Multiple Ranges", {
        {"[^a-zA-Z]", false, "NOT alphabetic"},
        {"[^a-z0-9]", false, "NOT alphanumeric lowercase"},
        {"[^A-Z0-9]", false, "NOT alphanumeric uppercase"},
        {"[^a-zA-Z0-9]", false, "NOT alphanumeric"},
        {"[^a-z\\d]", false, "NOT lowercase or digit (with char class)"},
        {"[^\\w]", false, "NOT word char"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/start_range
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/start_range - Dash at Start of Set", {
        {"[-]", false, "Just dash"},
        {"[-a]", false, "Dash then 'a'"},
        {"[-az]", false, "Dash, 'a', 'z'"},
        {"[-a-z]", false, "Dash, then range a-z"},
        {"[^-]", false, "NOT dash"},
        {"[^-a-z]", false, "NOT (dash or a-z)"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/end_range
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/end_range - Dash at End of Set", {
        {"[a-]", false, "'a' and dash"},
        {"[ab-]", false, "'a', 'b', dash"},
        {"[a-z-]", false, "Range a-z and dash"},
        {"[0-9-]", false, "Digits and dash"},
        {"[^a-]", false, "NOT ('a' or dash)"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/alternatives
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/alternatives - Alternation (|)", {
        {"a|b", false, "'a' OR 'b'"},
        {"a|b|c", false, "'a' OR 'b' OR 'c'"},
        {"ab|cd", false, "'ab' OR 'cd'"},
        {"abc|def|ghi", false, "Three alternatives"},
        {"a|", false, "'a' OR empty"},
        {"|a", false, "empty OR 'a'"},
        {"|", false, "empty OR empty"},
        {"a||b", false, "'a' OR empty OR 'b'"},
        {"(a|b)c", false, "Grouped alternation"},
        {"a(b|c)d", false, "Alternation in middle"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/1or5or9
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/1or5or9 - Specific Alternation Cases", {
        {"1|5|9", false, "Digits: 1 OR 5 OR 9"},
        {"[159]", false, "Character set: 1, 5, or 9"},
        {"1|5", false, "Two digit alternatives"},
        {"[15]", false, "Two digits in set"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/a_notbc_d
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/a_notbc_d - Negated Set in Pattern", {
        {"a[^bc]d", false, "'a', NOT ('b' or 'c'), 'd'"},
        {"a[^b]c", false, "'a', NOT 'b', 'c'"},
        {"[^a]b", false, "NOT 'a', then 'b'"},
        {"a[^a-z]", false, "'a', then NOT lowercase"},
        {"[^0-9]a", false, "NOT digit, then 'a'"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/groups
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/groups - Capturing Groups", {
        {"(a)", false, "Single char in group"},
        {"(ab)", false, "Two chars in group"},
        {"(abc)", false, "Three chars in group"},
        {"(a|b)", false, "Alternation in group"},
        {"(a)(b)", false, "Two groups"},
        {"(a)(b)(c)", false, "Three groups"},
        {"a(b)c", false, "Group in middle"},
        {"((a))", false, "Nested groups"},
        {"((a)(b))", false, "Nested groups with multiple inner"},
        {"(a*)", false, "Quantified inside group"},
        {"(a+b*)", false, "Multiple quantified inside"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/non_capturing_groups
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/non_capturing_groups - Non-Capturing Groups", {
        {"(?:a)", false, "Single char in non-capturing"},
        {"(?:ab)", false, "Two chars in non-capturing"},
        {"(?:a|b)", false, "Alternation in non-capturing"},
        {"(?:a)(?:b)", false, "Two non-capturing groups"},
        {"a(?:bc)d", false, "Non-capturing in middle"},
        {"(?:(?:a))", false, "Nested non-capturing"},
        {"(a)(?:b)", false, "Capturing and non-capturing mixed"},
        {"(?:a)(b)", false, "Non-capturing then capturing"},
        {"(?:a)*", false, "Non-capturing with quantifier"},
        {"(?:a|b)+", false, "Non-capturing alternation with plus"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Complex Patterns (Combinations)
    // ════════════════════════════════════════════════════════════════
    run_test_section("Complex Patterns - Real-World Examples", {
        {"[a-zA-Z_]\\w*", false, "Identifier pattern"},
        {"\\d+\\.\\d+", false, "Decimal number"},
        {"\\w+@\\w+\\.\\w+", false, "Simple email pattern"},
        {"[a-zA-Z0-9_]+", false, "Username pattern"},
        {"(a|b)*c+", false, "Kleene star and plus"},
        {"a(bc)*d", false, "Spec example"},
        {"(a+|b*)?", false, "Nested quantifiers with alternation"},
        {"[\\w\\s]+", false, "Word chars or spaces"},
        {"\\w+\\s*\\w*", false, "Word, optional space, optional word"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Character Set Edge Cases
    // ════════════════════════════════════════════════════════════════
    run_test_section("Character Set Edge Cases", {
        {"[*]", false, "Literal asterisk in set"},
        {"[+]", false, "Literal plus in set"},
        {"[?]", false, "Literal question in set"},
        {"[.]", false, "Literal dot in set"},
        {"[|]", false, "Literal pipe in set"},
        {"[(]", false, "Literal open paren in set"},
        {"[)]", false, "Literal close paren in set"},
        {"[*+?.]", false, "Multiple special chars in set"},
        {"[\\]]", false, "Escaped close bracket in set"},
        {"[a\\]]", false, "'a' and escaped bracket"},
        {"[\\]a]", false, "Escaped bracket and 'a'"},
        {"[^\\]]", false, "NOT close bracket"},
        {"[--/]", false, "Range from dash to slash"},
        {"[a-zA-Z0-9_-]", false, "Alphanumeric, underscore, dash"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Character Classes in Sets
    // ════════════════════════════════════════════════════════════════
    run_test_section("Character Classes Inside Sets", {
        {"[\\w]", false, "Word class in set"},
        {"[\\d]", false, "Digit class in set"},
        {"[\\s]", false, "Space class in set"},
        {"[\\W]", false, "Non-word class in set"},
        {"[\\D]", false, "Non-digit class in set"},
        {"[\\S]", false, "Non-space class in set"},
        {"[a\\w]", false, "'a' or word class"},
        {"[\\w\\d]", false, "Word or digit class"},
        {"[\\w\\s]", false, "Word or space class"},
        {"[a-z\\d]", false, "Lowercase range or digit class"},
        {"[^\\w]", false, "NOT word class"},
        {"[^\\d\\s]", false, "NOT (digit or space)"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // ERROR TESTS
    // ════════════════════════════════════════════════════════════════

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/error_quant
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/error_quant - Quantifier Errors", {
        {"*", true, "Quantifier without base"},
        {"*a", true, "Star at start"},
        {"+", true, "Plus alone"},
        {"+a", true, "Plus at start"},
        {"?", true, "Question alone"},
        {"?a", true, "Question at start"},
        {"a**", true, "Double star"},
        {"a++", true, "Double plus"},
        {"a??", true, "Double question"},
        {"a*+", true, "Star then plus"},
        {"a+*", true, "Plus then star"},
        {"a*?", true, "Star then question"},
        {"(*a)", true, "Star after open paren"},
        {"(+a)", true, "Plus after open paren"},
        {"(?a)", true, "Question after open paren"},
        {"|*", true, "Star after pipe"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/error_group
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/error_group - Group Errors", {
        {"(", true, "Unclosed group"},
        {"(a", true, "Unclosed group with content"},
        {"(ab", true, "Unclosed group with multiple chars"},
        {"((a)", true, "Nested unclosed"},
        {"(a(b)", true, "One unclosed in nested"},
        {")", true, "Unopened close paren"},
        {"a)", true, "Close paren after content"},
        {"(a))", true, "Extra close paren"},
        {"((a)))", true, "Extra close after nested"},
        {")(", true, "Reversed parens"},
        {")a(", true, "Close, content, open"},
        {"(?:", true, "Unclosed non-capturing"},
        {"(?:a", true, "Unclosed non-capturing with content"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: regex/error_alternative
    // ════════════════════════════════════════════════════════════════
    run_test_section("regex/error_alternative - Character Set Errors", {
        {"[", true, "Unclosed bracket"},
        {"[a", true, "Unclosed bracket with char"},
        {"[a-z", true, "Unclosed bracket with range"},
        {"[abc", true, "Unclosed bracket with chars"},
        {"[^", true, "Unclosed negated set"},
        {"[^a", true, "Unclosed negated set with char"},
        {"]", true, "Unopened close bracket"},
        {"a]", true, "Close bracket after content"},
        {"[]", true, "Empty set"},
        {"[^]", true, "Empty negated set"},
        {"[[a]", true, "Nested open bracket"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Invalid Characters
    // ════════════════════════════════════════════════════════════════
    run_test_section("Invalid Characters", {
        {"^", true, "Caret at start (anchor not supported)"},
        {"^a", true, "Caret before char"},
        {"a^b", true, "Caret in middle"},
        {"$", true, "Dollar sign (if not supported)"},
        {"a$", true, "Dollar at end"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Edge Cases
    // ════════════════════════════════════════════════════════════════
    run_test_section("Edge Cases", {
        {"()*", false, "Empty group with star"},
        {"()+", false, "Empty group with plus"},
        {"()?", false, "Empty group with question"},
        {"(|)*", false, "Empty alternatives with star"},
        {"(a|)*", false, "'a' or empty, with star"},
        {"(|a)*", false, "empty or 'a', with star"},
        {"((((a))))", false, "Deeply nested groups"},
        {"a|b|c|d|e|f|g|h", false, "Many alternatives"},
        {"(a(b(c(d))))", false, "Deep nesting with content"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Complex Edge Case Patterns
    // ════════════════════════════════════════════════════════════════

    run_test_section("Complex Pattern 1: [\\w]+.[^)-,u\\W\\s\\s]", {
        {"[\\w]+.[^)-,u\\W\\s\\s]", false, "Word chars + any char + NOT(specials/u/whitespace)"},
    }, result);

    std::cout << "\n  ┌─ Pattern Breakdown: [\\w]+.[^)-,u\\W\\s\\s]\n";
    std::cout << "  │\n";
    std::cout << "  │  Component 1: [\\w]+\n";
    std::cout << "  │  • Matches: One or more word characters (a-z, A-Z, 0-9, _)\n";
    std::cout << "  │\n";
    std::cout << "  │  Component 2: .\n";
    std::cout << "  │  • Matches: Any single character\n";
    std::cout << "  │\n";
    std::cout << "  │  Component 3: [^)-,u\\W\\s\\s]\n";
    std::cout << "  │  • Matches: Any character NOT in this set:\n";
    std::cout << "  │    - ')' (close paren)\n";
    std::cout << "  │    - '-' (dash)\n";
    std::cout << "  │    - ',' (comma)\n";
    std::cout << "  │    - 'u' (literal u)\n";
    std::cout << "  │    - \\W (non-word characters)\n";
    std::cout << "  │    - \\s (whitespace - duplicate but same)\n";
    std::cout << "  └─\n";

    run_test_section("Complex Pattern 2: (?:.|)*[]-]+", {
        {"(?:.|)*[]-]+", false, "(Any char OR empty)* followed by one or more ] or -"},
    }, result);

    std::cout << "\n  ┌─ Pattern Breakdown: (?:.|)*[]-]+\n";
    std::cout << "  │\n";
    std::cout << "  │  Component 1: (?:.|)*\n";
    std::cout << "  │  • Non-capturing group with alternation\n";
    std::cout << "  │  • Matches: (any single char OR empty), zero or more times\n";
    std::cout << "  │  • Essentially: anything or nothing\n";
    std::cout << "  │\n";
    std::cout << "  │  Component 2: []-]+\n";
    std::cout << "  │  • Character set containing: ']' and '-'\n";
    std::cout << "  │  • Note: ']' at start of set is literal\n";
    std::cout << "  │  • Note: '-' at end is literal (not range)\n";
    std::cout << "  │  • Matches: One or more ] or - at the END\n";
    std::cout << "  └─\n";

    run_test_section("Complex Pattern Variations", {
        {"[\\w]+.[^abc\\W]", false, "Variation: simpler negated set"},
        {"[\\d]+.[^0-5]", false, "Variation: digits + not(0-5)"},
        {"(?:a|)*[]-]+", false, "Variation: (a OR empty)* then []-]+"},
        {"(?:ab|cd)*[]-]+", false, "Variation: alternation with content"},
    }, result);

    run_test_section("Negated Sets with Character Classes", {
        {"[^\\w]", false, "NOT word char"},
        {"[^\\d]", false, "NOT digit"},
        {"[^\\s]", false, "NOT whitespace"},
        {"[^\\W]", false, "NOT non-word (double negative)"},
        {"[^\\d\\s]", false, "NOT (digit or space)"},
        {"[^a-z\\d]", false, "NOT (lowercase or digit)"},
        {"[abc\\W]", false, "a, b, c, OR non-word char"},
        {"[^abc\\W]", false, "NOT (a, b, c, or non-word)"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // TEST: Project Specification Example
    // ════════════════════════════════════════════════════════════════
    run_test_section("Project Specification Example - a(b|c)*d", {
        {"a(b|c)*d", false, "Spec example: 'a', (b OR c) zero or more times, then 'd'"},
    }, result);

    // ════════════════════════════════════════════════════════════════
    // FINAL REPORT
    // ════════════════════════════════════════════════════════════════

    std::cout << "\n\n";
    std::cout << "████████████████████████████████████████████████████████████████\n";
    std::cout << "██                                                            ██\n";
    std::cout << "██                      FINAL RESULTS                         ██\n";
    std::cout << "██                                                            ██\n";
    std::cout << "████████████████████████████████████████████████████████████████\n";

    const double pct = (result.total > 0) ? (100.0 * result.passed / result.total) : 0.0;
    std::cout << "\n  Total Tests:  " << result.total << "\n";
    std::cout << "  Passed:       " << result.passed << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
    std::cout << "  Failed:       " << result.failures.size() << "\n";

    if (!result.failures.empty()) {
        std::cout << "\n  ┌─ FAILED TESTS:\n";
        for (const auto& failure : result.failures) {
            std::cout << "  │ • " << failure << "\n";
        }
        std::cout << "  └─\n";
    }

    std::cout << "\n================================================================\n";

    if (result.passed == result.total) {
        std::cout << "\n  ✅ ALL TESTS PASSED! 🎉\n\n";
        return 0;
    } else {
        std::cout << "\n  ❌ SOME TESTS FAILED\n\n";
        return 1;
    }
}
