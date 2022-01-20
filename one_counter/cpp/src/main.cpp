
#include <semi_manual_teacher.h>
#include <automaton_teacher.h>
#include "language.h"
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

active_learning::R1CA get_anbam_ref(active_learning::basic_alphabet &alphabet) {
    std::set<size_t> final = {1};
    auto transitions = active_learning::R1CA::transition_func_t();
    transitions.insert(std::make_pair<active_learning::R1CA::transition_x, active_learning::R1CA::transition_y>({0, 0, 'a'}, {0, 1}));
    transitions.insert(std::make_pair<active_learning::R1CA::transition_x, active_learning::R1CA::transition_y>({0, 1, 'a'}, {0, 1}));
    transitions.insert(std::make_pair<active_learning::R1CA::transition_x, active_learning::R1CA::transition_y>({0, 1, 'b'}, {1, 0}));
    transitions.insert(std::make_pair<active_learning::R1CA::transition_x, active_learning::R1CA::transition_y>({1, 0, 'a'}, {1, 0}));
    transitions.insert(std::make_pair<active_learning::R1CA::transition_x, active_learning::R1CA::transition_y>({1, 1, 'a'}, {1, -1}));

    return active_learning::R1CA::from_scratch(0, 2, 0, final, transitions, alphabet);
}

void learn_v1ca(bool verbose) {
    std::map<char, int> symbols;
    symbols.insert({'a', 1});
    symbols.insert({'b', -1});
    active_learning::visibly_alphabet_t alphabet(symbols);

    auto teacher = active_learning::semi_manual_teacher(is_anbn, alphabet);
    auto learner = active_learning::learner(teacher, alphabet);

    auto res = learner.learn_V1CA(verbose);
    std::cout << teacher.sum_up_msg() << std::endl;

    res.display("res");
}

void learn_r1ca(bool verbose) {
    std::set<char> symbols = {'a' , 'b'};
    active_learning::basic_alphabet_t alphabet(symbols);

    auto ref = get_anbam_ref(alphabet);
    auto teacher = active_learning::automaton_teacher(ref);
    auto learner = active_learning::learner(teacher, alphabet);

    auto res = learner.learn_R1CA(verbose);
    std::cout << teacher.sum_up_msg() << std::endl;

    res.display("res");
}

int main() {
    learn_v1ca(true);

    return 0;
}
