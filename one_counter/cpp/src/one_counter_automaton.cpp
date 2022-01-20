#include "one_counter_automaton.h"

namespace active_learning {

    const std::set<char> &one_counter_automaton::get_alphabet_symbols() {
        return alphabet_.symbols();
    }

    one_counter_automaton::one_counter_automaton(alphabet &alphabet, displayable_type disp_type) :
                                                                displayable(disp_type), alphabet_(alphabet) {}

    bool one_counter_automaton::transition_x::operator==(const one_counter_automaton::transition_x &other) const {
        return other.state == state
               and other.counter == counter
               and other.symbol == symbol;
    }

    bool one_counter_automaton::transition_x::operator<(const one_counter_automaton::transition_x &other) const {

        if (state != other.state)
            return state > other.state;
        if (counter != other.counter)
            return counter > other.counter;

        return symbol > other.symbol;
    }
}
