#include "automatic_v1ca_teacher.h"
#include "language.h"
#include "behaviour_graph.h"

#include <utility>

namespace active_learning {

    std::optional<std::string>
    automatic_v1ca_teacher::partial_equivalence_query(behaviour_graph &behaviour_graph,
                                                      const std::string &path) {
        (void) path; // unused

        if (behaviour_graph.is_isomorphic_to(behaviour_ref_, 0, 10000, alphabet_))
            return std::nullopt;

        return "";  // TODO
    }

    std::optional<std::string>
    automatic_v1ca_teacher::equivalence_query(one_counter_automaton &automaton, const std::string &path) {
        (void) path;
        auto &v1ca = oca_to_v1ca(automaton);

        if (v1ca.is_equivalent_to(automaton_ref_))
            return std::nullopt;

        return find_counter_example(v1ca);
    }

    bool active_learning::automatic_v1ca_teacher::membership_query_(const std::string &word) {
        return automaton_ref_.accepts(word);
    }

    std::string automatic_v1ca_teacher::find_counter_example(V1CA &automaton) {
        // How about trying to get the counter example from empty() ??
        // Not sure it would perfectly work tho
        (void) automaton;
        std::string ce;
        // keep the following line
        if (alphabet_.get_cv(ce)) {
            throw std::runtime_error("Automatic teacher found a counter example whose cv is not 0.");
        }
        return std::string();
    }

    automatic_v1ca_teacher::automatic_v1ca_teacher(behaviour_graph &behaviourRef,
                                                   V1CA &automatonRef,
                                                   visibly_alphabet_t alphabet) :
            behaviour_ref_(behaviourRef), automaton_ref_(automatonRef), alphabet_(
            std::move(alphabet)) {}

    V1CA &automatic_v1ca_teacher::oca_to_v1ca(one_counter_automaton &v1ca) {
        auto *v1ca_ptr = dynamic_cast<V1CA *>(&v1ca);
        if (!v1ca_ptr)
            throw std::runtime_error("automatic V1CA teacher must be given a V1CA type.");

        return *v1ca_ptr;
    }

}