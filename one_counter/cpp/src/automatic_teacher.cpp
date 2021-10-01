#include "automatic_teacher.h"
#include "language.h"

#include <utility>

std::optional<std::string>
active_learning::automatic_teacher::partial_equivalence_query(active_learning::V1CA &behaviour_graph,
                                                              const std::string &path) {
    (void) path;
    if (behaviour_ref_.is_subset_of(behaviour_graph))
        return std::nullopt;

    return find_counter_example(behaviour_graph);
}

std::optional<std::string>
active_learning::automatic_teacher::equivalence_query(active_learning::V1CA &automaton, const std::string &path) {
    (void) path;
    if (automaton.is_equivalent_to(automaton_ref_))
        return std::nullopt;

    return find_counter_example(automaton);
}

bool active_learning::automatic_teacher::belong_query_(const std::string &word) {
    return check_func_(word);
}

std::string active_learning::automatic_teacher::find_counter_example(active_learning::V1CA &automaton) {
    // How about trying to get the counter example from empty() ??
    // Not sure it would perfectly work tho
    (void) automaton;
    std::string ce;
    // keep the following line
    if (get_cv(ce, alphabet_)) {
        throw std::runtime_error("Automatic teacher found a counter example whose cv is not 0.");
    }
    return std::string();
}

active_learning::automatic_teacher::automatic_teacher(std::function<bool(const std::string &)> checkFunc,
                                                      active_learning::V1CA &behaviourRef,
                                                      active_learning::V1CA &automatonRef,
                                                      active_learning::alphabet_t alphabet) : check_func_(std::move(
        checkFunc)), behaviour_ref_(behaviourRef), automaton_ref_(automatonRef), alphabet_(std::move(alphabet)) {}

