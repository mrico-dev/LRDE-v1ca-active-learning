#pragma once

#include "V1CA.h"
#include "R1CA.h"
#include "teachers/teacher.h"
#include "dataframe.h"
#include "teachers/automaton_teacher.h"

namespace active_learning {

    enum class learner_mode {
        UNINITIALIZED,
        V1CA,
        R1CA
    };

    class learner {

    private:
        bool make_rst_consistent(RST &rst);

        bool make_rst_closed(RST &rst);

        int get_cv(const std::string &word);

        int get_cv(char symbol);

        bool is_O_equivalent_(const std::string &word1, const std::string &word2, RST &rst);

        std::set<std::string> get_congruence_set_(const std::string &word, RST &rst);

    public:
        learner(teacher &teacher, alphabet &alphabet);

        V1CA learn_V1CA(bool verbose = false);

        R1CA learn_R1CA(bool verbose = false);

    private:
        teacher &teacher_;
        alphabet &alphabet_;
        visibly_alphabet_t *as_visibly_alphabet_;
        basic_alphabet_t *as_basic_alphabet_;
        automaton_teacher *as_automaton_teacher_;
        learner_mode mode_ = learner_mode::UNINITIALIZED;
    };

}

// CPP_V1CALEARNER_H
