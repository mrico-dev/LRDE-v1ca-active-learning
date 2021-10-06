#pragma once

#include "manual_eq_queries_teacher.h"

namespace active_learning {

    class semi_manual_teacher : public manual_eq_queries_teacher {

        bool belong_query_(const std::string &word) override;

    public:
        semi_manual_teacher(const std::function<bool(const std::string &)> &checkFunc, visibly_alphabet_t &alphabet);

    private:
        std::function<bool(const std::string &)> check_func;
    };

}

//V1C2AL_SEMI_MANUAL_TEACHER_H
