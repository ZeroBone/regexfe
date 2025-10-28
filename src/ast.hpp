#pragma once
#include <vector>

class AstNode {
public:
    virtual ~AstNode() = default;
};

class GroupOrMatch : public AstNode {};

class Conjunction final : public AstNode {
public:
    std::vector<GroupOrMatch*> children;

    explicit Conjunction(GroupOrMatch* el) {
        children.push_back(el);
    }
};

class Expression final : public AstNode {
public:
    std::vector<Conjunction*> children;

    explicit Expression(Conjunction* conj) {
        children.push_back(conj);
    }

    explicit Expression() = default;
};

class Group final : public GroupOrMatch {
public:
    bool is_noncapturing;
    Expression* expression;

    explicit Group(const bool is_noncapturing, Expression* expression): is_noncapturing(is_noncapturing), expression(expression) {}
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
    NormalNotIncludingClosingBracket
};

class CharacterRange final : public AstNode {
public:
    char lower_bound;
    char upper_bound;

    explicit CharacterRange(const char lower_bound, const char upper_bound): lower_bound(lower_bound), upper_bound(upper_bound) {}

    explicit CharacterRange(const char c): lower_bound(c), upper_bound(c) {}
};

class CharacterSet final : public AstNode {
public:
    std::vector<CharacterRange*> ranges;
    std::vector<CharacterClass> classes;

    explicit CharacterSet(CharacterRange* range) {
        ranges.push_back(range);
    }

    explicit CharacterSet(const CharacterClass& cls) {
        classes.push_back(cls);
    }
};

class CharacterAlt final : public AstNode {
public:
    CharacterAltType type;
    CharacterSet* set;

    explicit CharacterAlt(const CharacterAltType type, CharacterSet* set): type(type), set(set) {}
};

class MatchElement : public AstNode {};

class DotMatchElement final : public MatchElement {};

class LiteralMatchElement final : public MatchElement {
public:
    char value;

    explicit LiteralMatchElement(const char value): value(value) {}
};

class CharacterClassMatchElement final : public MatchElement {
public:
    CharacterClass char_class;

    explicit CharacterClassMatchElement(const CharacterClass char_class): char_class(char_class) {}
};

class CharacterAltMatchElement final : public MatchElement {
public:
    CharacterAlt* character_alt;

    explicit CharacterAltMatchElement(CharacterAlt* character_alt): character_alt(character_alt) {}
};

class MatchNode final : public GroupOrMatch {
public:
    MatchElement* element;
    Quantifier* quantifier;

    explicit MatchNode(MatchElement* el): element(el), quantifier(nullptr) {}

    explicit MatchNode(MatchElement* el, const Quantifier quantifier): element(el), quantifier(new Quantifier(quantifier)) {}
};