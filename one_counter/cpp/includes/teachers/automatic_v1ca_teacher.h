#pragma once

#include "teacher.h"
#include "V1CA.h"
#include "behaviour_graph.h"

namespace active_learning {

    class automatic_v1ca_teacher : public cached_teacher {
    public:

        automatic_v1ca_teacher(V1CA &automatonRef,
                               visibly_alphabet_t alphabet);

        std::optional<std::string>
        partial_equivalence_query(behaviour_graph &behaviour_graph, const std::string &path) override;

        std::optional<std::string>
        equivalence_query(one_counter_automaton &automaton, const std::string &path) override;

    protected:
        bool membership_query_(const std::string &word) override;

    private:
        static V1CA &oca_to_v1ca(one_counter_automaton &v1ca);

    private:
        std::unique_ptr<behaviour_graph> behaviour_ref_ = nullptr;
        V1CA &automaton_ref_;
        visibly_alphabet_t alphabet_;
    };

}

