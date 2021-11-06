#pragma once

#include "V1CA.h"
#include "R1CA.h"
#include "teacher.h"
#include "dataframe.h"

namespace active_learning {

    enum class learner_mode {
        V1CA,
        R1CA
    };

    class learner {

    private:
        bool make_rst_consistent(RST &rst);

        bool make_rst_closed(RST &rst);

        int get_cv(const std::string& word);

        int get_cv(char symbol);

        bool is_O_equivalent_(const std::string &word1, const std::string &word2, RST &rst);

        std::set<std::string> get_congruence_set_(const std::string &word, RST &rst);

    public:
        learner(teacher &teacher, visibly_alphabet_t &alphabet);
        learner(teacher &teacher, basic_alphabet &alphabet);

        V1CA learn_V1CA(bool verbose = false);
        R1CA learn_R1CA(bool verbose = false);

    private:
        teacher &teacher_;
        visibly_alphabet_t v_alphabet_;
        basic_alphabet b_alphabet_;
        learner_mode mode_;
    };

}

// CPP_V1CALEARNER_H
