#include "one_counter_automaton.h"

namespace active_learning {


    alphabet &one_counter_automaton::get_alphabet() {
        return alphabet_;
    }

    one_counter_automaton::one_counter_automaton(alphabet &alphabet, displayable_type disp_type) :
                                                                displayable(disp_type), alphabet_(alphabet) {}

}
