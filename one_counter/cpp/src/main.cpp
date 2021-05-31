
#include "language.h"
#include "manual_teacher.h"
#include "V1CA_learner.h"

int main() {

    active_learning::alphabet_t alphabet;
    alphabet.insert({'a', 1});
    alphabet.insert({'b', -1});

    auto teacher = active_learning::manual_teacher();

    auto learner = active_learning::V1CA_learner(teacher, alphabet);

    auto res = learner.learn_V1CA(true);

    res.display("res");
}
