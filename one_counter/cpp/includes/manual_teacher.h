#ifndef V1C2AL_MANUAL_TEACHER_H
#define V1C2AL_MANUAL_TEACHER_H

#include "manual_eq_queries_teacher.h"

namespace active_learning {

    class manual_teacher : public manual_eq_queries_teacher{

    protected:
        bool belong_query_(const std::string &word) override;

    };

}

#endif //V1C2AL_MANUAL_TEACHER_H
