#include "ast.hpp"

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
