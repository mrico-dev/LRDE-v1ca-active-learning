#include "semi_manual_teacher.h"

namespace active_learning {

    bool semi_manual_teacher::belong_query_(const std::string &word) {
        return check_func(word);
    }

}
