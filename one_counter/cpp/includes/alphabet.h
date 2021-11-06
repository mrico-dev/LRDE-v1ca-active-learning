#pragma once

#include "word_counter.h"

#include <set>
#include <map>

namespace active_learning {

    class alphabet {
    public:
        [[nodiscard]] virtual const std::set<char> &symbols() const = 0;
    };

    class basic_alphabet : public alphabet {
    public:
        bool contains(char symbol);

        [[nodiscard]] const std::set<char> &symbols() const override;

    private:
        std::set<char> symbols_;
    };

    class visibly_alphabet : public alphabet, public word_counter {
    public:
        explicit visibly_alphabet(const std::map<char, int>& symbolsAndValues);

        visibly_alphabet();

        bool contains(char symbol);

        int get_cv(char symbol);

        int get_cv(const std::string &word) override;

        [[nodiscard]] const std::set<char> &symbols() const override;

    private:
        std::set<char> symbols_;
        std::map<char, int> symbols_and_values_;
    };

    // Aliases
    using basic_alphabet_t = basic_alphabet;
    using visibly_alphabet_t = visibly_alphabet;
}
