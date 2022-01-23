#include "teachers/semi_manual_teacher.h"

#include <utility>

namespace active_learning {

    bool semi_manual_teacher::membership_query_(const std::string &word) {
        return check_func(word);
    }

    semi_manual_teacher::semi_manual_teacher(const std::function<bool(const std::string &)> &checkFunc,
                                             visibly_alphabet_t &alphabet) {
        alphabet_ = alphabet;
        check_func = checkFunc;
    }

}
