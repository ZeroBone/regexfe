#include "ast.hpp"

MimRegex Conjunction::generateMimIR(MimirCodeGen& code_gen) const {

    if (children.empty()) {
        return code_gen.regex_empty();
    }

    if (children.size() == 1) {
        return children[0]->generateMimIR(code_gen);
    }

    std::vector<MimRegex> children_regexes;

    for (const Match* child : children) {
        children_regexes.push_back(child->generateMimIR(code_gen));
    }

    return code_gen.regex_conj(children_regexes);

}

MimRegex Expression::generateMimIR(MimirCodeGen& code_gen) const {

    if (children.empty()) {
        return code_gen.regex_empty();
    }

    if (children.size() == 1) {
        return children[0]->generateMimIR(code_gen);
    }

    std::vector<MimRegex> regexes;
    for (const Conjunction* conj : children) {
        regexes.push_back(conj->generateMimIR(code_gen));
    }

    return code_gen.regex_disj(regexes);

}

MimRegex characterClassToRegex(MimirCodeGen& code_gen, const CharacterClass cls) {

    switch (cls) {

        case CharacterClass::WordChars: {
            const MimRegex az = code_gen.regex_range(code_gen.char_lit('a'), code_gen.char_lit('z'));
            const MimRegex az_cap = code_gen.regex_range(code_gen.char_lit('A'), code_gen.char_lit('Z'));
            const MimRegex digits = code_gen.regex_range(code_gen.char_lit('0'), code_gen.char_lit('9'));
            const MimRegex underscore = code_gen.regex_lit('_');
            return code_gen.regex_disj({az, az_cap, digits, underscore});
        }

        case CharacterClass::NonWordChars: {
            const MimRegex r1 = code_gen.regex_range(code_gen.char_lit(0x00), code_gen.char_lit(0x2f));
            const MimRegex r2 = code_gen.regex_range(code_gen.char_lit(0x3a), code_gen.char_lit(0x40));
            const MimRegex r3 = code_gen.regex_range(code_gen.char_lit(0x5b), code_gen.char_lit(0x5e));
            const MimRegex r4 = code_gen.regex_lit(0x60);
            const MimRegex r5 = code_gen.regex_range(code_gen.char_lit(0x7b), code_gen.char_lit(0x7f));
            return code_gen.regex_disj({r1, r2, r3, r4, r5});
        }

        case CharacterClass::DigitChars:
            return code_gen.regex_range(code_gen.char_lit('0'), code_gen.char_lit('9'));

        case CharacterClass::NonDigitChars: {
            const MimRegex r1 = code_gen.regex_range(code_gen.char_lit(0x00), code_gen.char_lit(0x2f));
            const MimRegex r2 = code_gen.regex_range(code_gen.char_lit(0x3a), code_gen.char_lit(0x7f));
            return code_gen.regex_disj({r1, r2});
        }

        case CharacterClass::WhiteSpaceChars:
            return code_gen.regex_disj({
                code_gen.regex_lit(' '),
                code_gen.regex_lit('\n'),
                code_gen.regex_lit('\r'),
                code_gen.regex_lit('\t'),
                code_gen.regex_lit('\v'),
                code_gen.regex_lit('\f')
            });

        case CharacterClass::NonWhiteSpaceChars: {
            const MimRegex r1 = code_gen.regex_range(code_gen.char_lit(0x00), code_gen.char_lit(0x08));
            const MimRegex r2 = code_gen.regex_range(code_gen.char_lit(0x0e), code_gen.char_lit(0x1f));
            const MimRegex r3 = code_gen.regex_range(code_gen.char_lit(0x21), code_gen.char_lit(0x7f));
            return code_gen.regex_disj({r1, r2, r3});
        }

        default:
            assert(false);
    }
}

MimRegex CharacterSet::generateMimIR(MimirCodeGen& code_gen, const bool negate, const bool addClosingBracket) const {

    std::vector<MimRegex> regexes;

    if (addClosingBracket) {
        regexes.push_back(code_gen.regex_lit(']'));
    }

    for (const CharacterRange* range : ranges) {
        regexes.push_back(range->generateMimIR(code_gen));
    }

    for (const CharacterClass cls : classes) {
        regexes.push_back(characterClassToRegex(code_gen, cls));
    }

    assert(!regexes.empty());

    const MimRegex result = code_gen.regex_disj(regexes);
    return negate ? code_gen.regex_not(result) : result;

}

MimRegex CharacterClassMatchElement::generateMimIR(MimirCodeGen& code_gen) const {
    return characterClassToRegex(code_gen, char_class);
}

MimRegex CharacterAlt::generateMimIR(MimirCodeGen& code_gen) const {

    if (set == nullptr) {
        switch (type) {
            case CharacterAltType::NormalIncludingClosingBracket:
                return code_gen.regex_lit(']');
            case CharacterAltType::NegatedIncludingClosingBracket:
                return code_gen.regex_not(code_gen.regex_lit(']'));
            default:
                assert(false);
        }
    }

    const bool negated_mode = type == CharacterAltType::Negated || type == CharacterAltType::NegatedIncludingClosingBracket;
    const bool include_closing_bracket = type == CharacterAltType::NormalIncludingClosingBracket || type == CharacterAltType::NegatedIncludingClosingBracket;

    return set->generateMimIR(code_gen, negated_mode, include_closing_bracket);

}

MimRegex Match::generateMimIR(MimirCodeGen& code_gen) const {

    const MimRegex elementRegex = element->generateMimIR(code_gen);

    if (quantifier == nullptr) {
        return elementRegex;
    }

    switch (*quantifier) {
        case Quantifier::Star:
            return code_gen.regex_star(elementRegex);
        case Quantifier::Plus:
            return code_gen.regex_plus(elementRegex);
        case Quantifier::QuestionMark:
            return code_gen.regex_optional(elementRegex);
        default:
            assert(false);
    }

}
