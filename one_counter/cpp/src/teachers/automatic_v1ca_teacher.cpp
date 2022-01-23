#include "teachers/automatic_v1ca_teacher.h"
#include "language.h"
#include "behaviour_graph.h"

#include <utility>

namespace active_learning {

    std::optional<std::string>
    automatic_v1ca_teacher::partial_equivalence_query(behaviour_graph &behaviour_graph,
                                                      const std::string &path) {
        (void) path; // unused
        auto basic_v1ca = behaviour_graph.to_v1ca_direct((visibly_alphabet_t &) automaton_ref_.get_alphabet());

        return automaton_ref_.is_equivalent_to(basic_v1ca);
    }

    std::optional<std::string>
    automatic_v1ca_teacher::equivalence_query(one_counter_automaton &automaton, const std::string &path) {
        (void) path; // unused
        auto &v1ca = oca_to_v1ca(automaton);

        return automaton_ref_.is_equivalent_to(v1ca);
    }

    bool active_learning::automatic_v1ca_teacher::membership_query_(const std::string &word) {
        return automaton_ref_.accepts(word);
    }

    automatic_v1ca_teacher::automatic_v1ca_teacher(V1CA &automatonRef,
                                                   visibly_alphabet_t alphabet) :
            automaton_ref_(automatonRef), alphabet_(
            std::move(alphabet)) {
        behaviour_ref_ = std::make_unique<behaviour_graph>(behaviour_graph::from_v1ca(automatonRef));
    }

    V1CA &automatic_v1ca_teacher::oca_to_v1ca(one_counter_automaton &v1ca) {
        auto *v1ca_ptr = dynamic_cast<V1CA *>(&v1ca);
        if (!v1ca_ptr)
            throw std::runtime_error("automatic V1CA teacher must be given a V1CA type.");

        return *v1ca_ptr;
    }

}