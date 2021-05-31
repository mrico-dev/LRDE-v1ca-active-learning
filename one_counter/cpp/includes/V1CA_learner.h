#ifndef CPP_V1CALEARNER_H
#define CPP_V1CALEARNER_H

#include "V1CA.h"
#include "teacher.h"
#include "dataframe.h"
#include "V1CA_builder.h"

namespace active_learning {

    class V1CA_learner {

    private:
        bool make_rst_consistent(RST rst);

        bool make_rst_closed(RST rst);

    public:
        V1CA_learner(teacher &teacher, alphabet_t &alphabet);

        V1CA learn_V1CA(bool verbose=false);

    private:
        teacher &teacher_;
        alphabet_t &alphabet_;
    };

}

#endif // CPP_V1CALEARNER_H
