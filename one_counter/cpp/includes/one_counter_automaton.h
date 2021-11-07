#pragma once

#include <boost/graph/adjacency_list.hpp>
#include "alphabet.h"
#include "displayable.h"

namespace active_learning {

    // TODO later when everything works
    class one_counter_automaton : public displayable {

    protected:
    public:
        one_counter_automaton(alphabet &alphabet, displayable_type disp_type);

    protected:
        virtual void to_delete_later() {}; // So the class is polymorphic

        alphabet &get_alphabet();

    protected:
        alphabet &alphabet_;
    };
}