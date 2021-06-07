
#include <semi_manual_teacher.h>
#include "language.h"
#include "manual_teacher.h"
#include "V1CA_learner.h"

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

int main() {

    active_learning::alphabet_t alphabet;
    alphabet.insert({'a', 1});
    alphabet.insert({'b', -1});

    auto teacher = active_learning::semi_manual_teacher(is_anbn, alphabet);

    auto learner = active_learning::V1CA_learner(teacher, alphabet);

    auto res = learner.learn_V1CA(true);
    std::cout << teacher.sum_up_msg() << std::endl;

    res.display("res");
}
