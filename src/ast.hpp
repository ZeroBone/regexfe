#pragma once
#include <vector>

#include <mim/def.h>
#include <mim/world.h>
#include <mim/plug/regex/regex.h>

class AstNode {
public:
    virtual ~AstNode() = default;
};

class GroupOrMatch : public AstNode {};

class Conjunction final : public AstNode {
    std::vector<GroupOrMatch*> children;
public:
    explicit Conjunction(GroupOrMatch* el) {
        children.push_back(el);
    }

    void add_child(GroupOrMatch* el) {
        children.push_back(el);
    }

    const mim::Def* generateMimIR(mim::World& world) const {
        return world.call<mim::plug::regex::lit>(world.lit_bool(true));
    }
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

    const mim::Def* generateMimIR(mim::World& world) const {

        assert(!children.empty());

        const mim::Def* cur_ir = children[0]->generateMimIR(world);

        for (size_t i = 1; i < children.size(); i++) {
            const mim::Def* cur = children[i]->generateMimIR(world);
            cur_ir = world.call<mim::plug::regex::disj>(cur_ir, cur);
        }

        return cur_ir;
    }

};

class Group final : public GroupOrMatch {

    bool is_noncapturing;
    Expression* expression;
public:
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
    char lower_bound;
    char upper_bound;
public:
    explicit CharacterRange(const char lower_bound, const char upper_bound): lower_bound(lower_bound), upper_bound(upper_bound) {}

    explicit CharacterRange(const char c): lower_bound(c), upper_bound(c) {}
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

    void add_character_class(CharacterClass cls) {
        classes.push_back(cls);
    }
};

class CharacterAlt final : public AstNode {

    CharacterAltType type;
    CharacterSet* set;
public:
    explicit CharacterAlt(const CharacterAltType type, CharacterSet* set): type(type), set(set) {}
};

class MatchElement : public AstNode {};

class DotMatchElement final : public MatchElement {};

class LiteralMatchElement final : public MatchElement {
    char value;
public:
    explicit LiteralMatchElement(const char value): value(value) {}
};

class CharacterClassMatchElement final : public MatchElement {
    CharacterClass char_class;
public:
    explicit CharacterClassMatchElement(const CharacterClass char_class): char_class(char_class) {}
};

class CharacterAltMatchElement final : public MatchElement {
    CharacterAlt* character_alt;
public:
    explicit CharacterAltMatchElement(CharacterAlt* character_alt): character_alt(character_alt) {}
};

class MatchNode final : public GroupOrMatch {
    MatchElement* element;
    Quantifier* quantifier;
public:
    explicit MatchNode(MatchElement* el): element(el), quantifier(nullptr) {}

    explicit MatchNode(MatchElement* el, const Quantifier quantifier): element(el), quantifier(new Quantifier(quantifier)) {}
};