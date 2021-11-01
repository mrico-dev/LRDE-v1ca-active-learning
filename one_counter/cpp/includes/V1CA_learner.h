#pragma once

#include "V1CA.h"
#include "teacher.h"
#include "dataframe.h"

namespace active_learning {

    class V1CA_learner {

    private:
        bool make_rst_consistent(RST &rst);

        bool make_rst_closed(RST &rst);

    public:
        V1CA_learner(teacher &teacher, visibly_alphabet_t &alphabet);

        V1CA learn_V1CA(bool verbose = false);

    private:
        teacher &teacher_;
        visibly_alphabet_t &alphabet_;
    };

}

// CPP_V1CALEARNER_H
