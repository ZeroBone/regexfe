#pragma once
#include <vector>

#include "mimir_codegen.hpp"

class AstNode {
public:
    virtual ~AstNode() = default;
};

class GroupOrMatch : public AstNode {

public:
    virtual MimRegex generateMimIR(MimirCodeGen& code_gen) const = 0;

};

class Conjunction final : public AstNode {
    std::vector<GroupOrMatch*> children;
public:
    explicit Conjunction(GroupOrMatch* el) {
        children.push_back(el);
    }

    void add_child(GroupOrMatch* el) {
        children.push_back(el);
    }

    MimRegex generateMimIR(MimirCodeGen& code_gen) const;
};

class Expression final : public AstNode {
    std::vector<Conjunction*> children;
public:
    explicit Expression(Conjunction* conj) {
        children.push_back(conj);
    }

    explicit Expression() = default;

    void add_child(Conjunction* conj) {
        children.push_back(conj);
    }

    MimRegex generateMimIR(MimirCodeGen& code_gen) const;

};

class Group final : public GroupOrMatch {

    const bool is_noncapturing;
    const Expression* const expression;

public:
    explicit Group(const bool is_noncapturing, const Expression* expression): is_noncapturing(is_noncapturing), expression(expression) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const override {
        // TODO: here we drop the information about whether the group is capturing or not, think about how to pass it into MimIR
        return expression->generateMimIR(code_gen);
    }

};

enum class Quantifier {
    Star,
    Plus,
    QuestionMark
};

enum class CharacterClass {
    WordChars,
    NonWordChars,
    DigitChars,
    NonDigitChars,
    WhiteSpaceChars,
    NonWhiteSpaceChars
};

enum class CharacterAltType {
    Normal,
    Negated,
    NormalIncludingClosingBracket,
    NegatedIncludingClosingBracket
};

class CharacterRange final : public AstNode {
    char lower_bound;
    char upper_bound;
public:
    explicit CharacterRange(const char lower_bound, const char upper_bound): lower_bound(lower_bound), upper_bound(upper_bound) {}

    explicit CharacterRange(const char c): lower_bound(c), upper_bound(c) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const {
        const MimChar lower_bound_char = code_gen.char_lit(lower_bound);
        const MimChar upper_bound_char = code_gen.char_lit(upper_bound);
        return code_gen.regex_range(lower_bound_char, upper_bound_char);
    }
};

class CharacterSet final : public AstNode {

    std::vector<CharacterRange*> ranges;
    std::vector<CharacterClass> classes;
public:
    explicit CharacterSet(CharacterRange* range) {
        ranges.push_back(range);
    }

    explicit CharacterSet(const CharacterClass& cls) {
        classes.push_back(cls);
    }

    void add_range(CharacterRange* range) {
        ranges.push_back(range);
    }

    void add_character_class(const CharacterClass cls) {
        classes.push_back(cls);
    }

    MimRegex generateMimIR(MimirCodeGen& code_gen, bool negate, bool addClosingBracket) const;

};

class CharacterAlt final : public AstNode {

    const CharacterAltType type;
    const CharacterSet* const set;

public:
    explicit CharacterAlt(const CharacterAltType type, const CharacterSet* set): type(type), set(set) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const;

};

class MatchElement : public AstNode {

public:
    virtual MimRegex generateMimIR(MimirCodeGen& code_gen) const = 0;

};

class DotMatchElement final : public MatchElement {

public:
    MimRegex generateMimIR(MimirCodeGen& code_gen) const override {
        return code_gen.regex_any();
    }

};

class LiteralMatchElement final : public MatchElement {

    char value;

public:
    explicit LiteralMatchElement(const char value): value(value) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const override {
        return code_gen.regex_lit(value);
    }

};

class CharacterClassMatchElement final : public MatchElement {

    const CharacterClass char_class;

public:
    explicit CharacterClassMatchElement(const CharacterClass char_class): char_class(char_class) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const override;

};

class CharacterAltMatchElement final : public MatchElement {

    const CharacterAlt* const character_alt;

public:
    explicit CharacterAltMatchElement(const CharacterAlt* character_alt): character_alt(character_alt) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const override {
        return character_alt->generateMimIR(code_gen);
    }

};

class MatchNode final : public GroupOrMatch {

    const MatchElement* const element;
    const Quantifier* const quantifier;

public:
    explicit MatchNode(const MatchElement* el): element(el), quantifier(nullptr) {}

    explicit MatchNode(const MatchElement* el, const Quantifier quantifier): element(el), quantifier(new Quantifier(quantifier)) {}

    MimRegex generateMimIR(MimirCodeGen& code_gen) const override;

};