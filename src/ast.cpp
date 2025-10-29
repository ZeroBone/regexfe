#include "ast.hpp"

MimRegex Conjunction::generateMimIR(MimirCodeGen& code_gen) const {

    assert(!children.empty());

    std::vector<MimRegex> children_regexes;

    for (const GroupOrMatch* child : children) {
        children_regexes.push_back(child->generateMimIR(code_gen));
    }

    return code_gen.regex_conj(children_regexes);

}

MimRegex Expression::generateMimIR(MimirCodeGen& code_gen) const {

    if (children.empty()) {
        return code_gen.regex_empty();
    }

    std::vector<MimRegex> regexes;
    for (const Conjunction* conj : children) {
        regexes.push_back(conj->generateMimIR(code_gen));
    }

    return code_gen.regex_disj(regexes);

}

MimRegex characterClassToRegex(MimirCodeGen& code_gen, const CharacterClass cls) {
    switch (cls) {
        case CharacterClass::WordChars:
            return code_gen.regex_class(cls::w);
        case CharacterClass::NonWordChars:
            return code_gen.regex_class(cls::W);
        case CharacterClass::DigitChars:
            return code_gen.regex_class(cls::d);
        case CharacterClass::NonDigitChars:
            return code_gen.regex_class(cls::D);
        case CharacterClass::WhiteSpaceChars:
            return code_gen.regex_class(cls::s);
        case CharacterClass::NonWhiteSpaceChars:
            return code_gen.regex_class(cls::S);
        default:
            assert(false);
    }
}

MimRegex CharacterSet::generateMimIR(MimirCodeGen& code_gen, const bool as_negated_conjunction) const {

    std::vector<MimRegex> regexes;

    for (const CharacterRange* range : ranges) {
        MimRegex range_regex = range->generateMimIR(code_gen);
        if (as_negated_conjunction) {
            range_regex = code_gen.regex_not(range_regex);
        }
        regexes.push_back(range_regex);
    }

    for (const CharacterClass cls : classes) {
        MimRegex class_regex = characterClassToRegex(code_gen, cls);
        if (as_negated_conjunction) {
            class_regex = code_gen.regex_not(class_regex);
        }
        regexes.push_back(class_regex);
    }

    assert(!regexes.empty());

    if (as_negated_conjunction) {
        return code_gen.regex_conj(regexes);
    }

    return code_gen.regex_disj(regexes);

}

MimRegex CharacterClassMatchElement::generateMimIR(MimirCodeGen& code_gen) const {
    return characterClassToRegex(code_gen, char_class);
}

MimRegex CharacterAlt::generateMimIR(MimirCodeGen& code_gen) const {

    const bool negated_conjunction_mode = type == CharacterAltType::Negated || type == CharacterAltType::NegatedIncludingClosingBracket;
    const bool include_closing_bracket = type == CharacterAltType::NormalIncludingClosingBracket || type == CharacterAltType::NegatedIncludingClosingBracket;

    MimRegex result = set->generateMimIR(code_gen, negated_conjunction_mode);

    if (include_closing_bracket) {
        const MimRegex closing_bracket = code_gen.regex_lit(']');

        std::vector<MimRegex> regexes;
        regexes.push_back(result);

        if (negated_conjunction_mode) {
            regexes.push_back(code_gen.regex_not(closing_bracket));
            result = code_gen.regex_conj(regexes);
        }
        else {
            regexes.push_back(closing_bracket);
            result = code_gen.regex_disj(regexes);
        }

    }

    return result;

}

MimRegex MatchNode::generateMimIR(MimirCodeGen& code_gen) const {

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
