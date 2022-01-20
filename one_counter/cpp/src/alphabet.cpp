#include "alphabet.h"

#include <utility>
#include <iostream>

namespace active_learning {
    bool basic_alphabet::contains(char symbol) const {
        return symbols_.contains(symbol);
    }

    const std::set<char> &basic_alphabet::symbols() const {
        return symbols_;
    }

    basic_alphabet::basic_alphabet(std::set<char> symbols) : symbols_(std::move(symbols)) {}

    bool visibly_alphabet::contains(char symbol) const {
        return symbols_and_values_.contains(symbol);
    }

    int visibly_alphabet::get_cv(char symbol) const {
        return symbols_and_values_.at(symbol);
    }

    int visibly_alphabet::get_cv(const std::string &word) const {
        int res = 0;
        for (char c : word)
            res += symbols_and_values_.at(c);

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

    bool visibly_alphabet::operator==(const visibly_alphabet &other) const {
        return other.symbols_and_values_ == symbols_and_values_;
    }

}
