#pragma once

#include "R1CA.h"
#include "R1CA_builder.h"
#include "dataframe.h"

namespace active_learning {

    class R1CA_learner {

    private:
        bool make_rst_consistent(RST &rst);

        bool make_rst_closed(RST &rst);

    public:
        R1CA_learner(automaton_teacher &teacher, alphabet_t &alphabet);

        R1CA learn_R1CA(bool verbose = false);

    private:
        automaton_teacher &teacher_;
        alphabet_t &alphabet_;
    };

}
