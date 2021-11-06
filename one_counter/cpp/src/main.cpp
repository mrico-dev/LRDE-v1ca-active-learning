
#include <semi_manual_teacher.h>
#include "language.h"
#include "manual_teacher.h"
#include "learner.h"

/**
 * @return true if the word is in the language {a^n.b^n}
 */
bool is_anbn(const std::string &word) {
    auto i = 0u;
    while (i < word.size() and word[i] == 'a')
        ++i;

    auto supposed_length = 2 * i;
    if (supposed_length != word.size())
        return false;

    while (i < supposed_length) {
        if (word[i] != 'b')
            return false;
        ++i;
    }

    return i == supposed_length;
}

/**
 * @return true if the word is in the language {x*.a^n.y*.b^n.z*}
 */
bool is_xanybnz(const std::string &word) {
    auto i = 0u;
    while (i < word.size() and word[i] == 'x')
        ++i;

    auto n1 = 0u;
    while (i < word.size() and word[i] == 'a') {
        ++i;
        ++n1;
    }

    while (i < word.size() and word[i] == 'y')
        ++i;

    auto n2 = 0u;
    while (i < word.size() and word[i] == 'b') {
        ++i;
        ++n2;
    }

    while (i < word.size() and word[i] == 'z')
        ++i;

    return n1 == n2 and i == word.size();
}

int main() {
    std::map<char, int> symbols;
    symbols.insert({'a', 1});
    symbols.insert({'b', -1});
    active_learning::visibly_alphabet_t alphabet(symbols);

    auto teacher = active_learning::semi_manual_teacher(is_anbn, alphabet);
    auto learner = active_learning::learner(teacher, alphabet);

    auto res = learner.learn_V1CA(true);
    std::cout << teacher.sum_up_msg() << std::endl;

    res.display("res");

    return 0;
}
