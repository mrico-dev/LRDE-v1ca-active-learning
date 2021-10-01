#include "language.h"

#include <iostream>

namespace active_learning {

    /**
     * Get the counter value of a word.
     * The counter value is the sum of all counter value of each symbol of the word
     * @param word The word whose counter value needs to be processed
     * @param alphabet The reference target language alphabet
     * @return The counter value of word
     */
    int get_cv(const std::string &word, alphabet_t &alphabet) {

        int res = 0;
        for (char c : word) {
            res += alphabet[c];
        }

        return res;
    }

    /**
     * Returns whether two words are O_equivalent according to the RST.
     * Two words a O_equivalent if they have the same counter value, and
     * if they accept the same language for any prefix added to them,
     * i.e if they have identical columns in the RST table.
     * @param word1 The first word
     * @param word2 The second word
     * @param rst The RST used as reference to tell if the words are O_equivalent
     * @param teacher A teacher that might be used for membership query if one of the word is not a label in the RST table.
     * @param alphabet The reference target language alphabet
     * @return true if the words are O_equivalent, false otherwise.
     */
    bool is_O_equivalent(const std::string &word1, const std::string &word2, RST &rst, teacher &teacher, alphabet_t &alphabet) {
        if (word1 == word2)
            return true;

        int cv_w = get_cv(word1, alphabet);
        if (cv_w != get_cv(word2, alphabet))
            return false;

        auto rst_copy = RST(rst);
        rst_copy.add_row_using_query_if_not_present(word1, cv_w, teacher, "is_O_equivalent");
        rst_copy.add_row_using_query_if_not_present(word2, cv_w, teacher, "is_O_equivalent");

        return rst_copy.compare_rows(word1, word2, cv_w);
    }

    /**
     * Get every word in the RST that are O_equivalent to word, including word itself.
     * @param word The word whose congruence set needs to be processed.
     * @param rst The RST used to compute O_equivalence.
     * @param teacher A teacher that might be used for membership query
     * @param alphabet The reference target language alphabet
     * @return The set of all words that are O_equivalent to word
     */
    std::set<std::string> get_congruence_set(const std::string &word, RST &rst, teacher &teacher, alphabet_t alphabet) {
        int cv_w = get_cv(word, alphabet);

        if (cv_w > static_cast<int>(rst.size())) {
            throw std::runtime_error("get_congruence_set(): given cv is out of bound of RST.");
        }

        auto res = std::set<std::string>();
        for (auto &table : rst.get_tables()) {
            for (auto &label : table.get_row_labels()) {
                if (is_O_equivalent(word, label, rst, teacher, alphabet)) {
                    res.insert(label);
                }
            }
        }

        return res;
    }

    /**
     * Returns all prefixes of a word, including the word itself and ""
     * @param word The word whose prefixes need to be computed
     * @return The vector of prefixes of the word
     */
    std::vector<std::string> get_all_prefixes(const std::string &word) {
        std::vector<std::string> res({""});
        std::string pref;
        for (char c : word) {
           pref.push_back(c);
           res.emplace_back(pref);
        }

        return res;
    }

    /**
     * Tell whether a word can be formed with the symbols of a specified alphabet,
     * i.e if all symbols of the word are present in the alphabet.
     * @param word The word to be verified
     * @param alphabet The reference alphabet
     * @return true if the word can be formed using the alphabet, false otherwise.
     */
    bool is_from_alphabet(const std::string &word, const alphabet_t &alphabet) {
        for (const char &c : word) {
            if (!alphabet.contains(c)) {
                return false;
            }
        }

        return true;
    }
}
