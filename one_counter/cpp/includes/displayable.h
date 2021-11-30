#pragma once

namespace active_learning {

    enum class displayable_type {
        V1CA,
        R1CA,
        behaviour_graph
    };

    class displayable {

    public:
        explicit displayable(displayable_type type) : displayable_type_(type) {}

        virtual void display(const std::string &path) = 0;

        displayable_type get_displayable_type() { return displayable_type_; }

    private:
        displayable_type displayable_type_;
    };

}
