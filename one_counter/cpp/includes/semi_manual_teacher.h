#ifndef V1C2AL_SEMI_MANUAL_TEACHER_H
#define V1C2AL_SEMI_MANUAL_TEACHER_H


#include "manual_eq_queries_teacher.h"

namespace active_learning {

    class semi_manual_teacher : public manual_eq_queries_teacher {

        bool belong_query_(const std::string &word) override;

        std::function<bool(const std::string&)> check_func;
    };

}

#endif //V1C2AL_SEMI_MANUAL_TEACHER_H
