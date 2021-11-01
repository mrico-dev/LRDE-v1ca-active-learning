#include "alphabet.h"

#include <utility>
#include <iostream>

namespace active_learning {
    bool basic_alphabet::contains(char symbol) {
        return symbols_.contains(symbol);
    }

    const std::set<char> &basic_alphabet::symbols() const {
        return symbols_;
    }

    bool visibly_alphabet::contains(char symbol) {
        return symbols_and_values_.contains(symbol);
    }

    int visibly_alphabet::get_cv(char symbol) {
        return symbols_and_values_[symbol];
    }

    int visibly_alphabet::get_cv(const std::string &word) {
        int res = 0;
        for (char c : word)
            res += symbols_and_values_[c];

        return res;
    }

    visibly_alphabet::visibly_alphabet(const std::map<char, int>& symbolsAndValues) {
        symbols_and_values_ = symbolsAndValues;
        for (auto c : symbols_and_values_) {
            symbols_.insert(c.first);
        }
    }

    const std::set<char> &visibly_alphabet::symbols() const {
        return symbols_;
    }

    visibly_alphabet::visibly_alphabet() = default;

}
