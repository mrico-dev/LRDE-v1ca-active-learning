#pragma once

#include "manual_eq_queries_teacher.h"

namespace active_learning {

    class manual_teacher : public manual_eq_queries_teacher {

    public:
        manual_teacher(alphabet_t &alphabet);

    protected:
        bool belong_query_(const std::string &word) override;

    };

}

//V1C2AL_MANUAL_TEACHER_H
