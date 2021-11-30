#include "learner.h"
#include "dataframe.h"
#include "behaviour_graph.h"
#include "language.h"

#include <iostream>
#include <automaton_teacher.h>

namespace active_learning {

    learner::learner(teacher &teacher, alphabet &alphabet) : teacher_(teacher), alphabet_(alphabet) {
        as_visibly_alphabet_ = dynamic_cast<visibly_alphabet_t*>(&alphabet);
        as_basic_alphabet_ = dynamic_cast<basic_alphabet_t*>(&alphabet);
        as_automaton_teacher_ = dynamic_cast<automaton_teacher*>(&teacher);
    }

    /**
     * Make the RST consistent
     * The algorithm is the following (TODO)
     * @param rst The RST
     * @return true if the RST was already consistent, false if a change was made
     */
    bool learner::make_rst_consistent(RST &rst) {
        for (auto &table : rst.get_tables()) {
            for (auto u_i = 0u; u_i < table.get_row_labels().size(); ++u_i) {
                const std::string &u = table.get_row_labels()[u_i];
                for (auto v_i = u_i + 1; v_i < table.get_row_labels().size(); ++v_i) {
                    const std::string &v = table.get_row_labels()[v_i];
                    if (is_O_equivalent_(u, v, rst)) {
                        for (auto c : alphabet_.symbols()) {
                            std::string uc = u + c;
                            std::string vc = v + c;
                            auto cv_uc = get_cv(uc);
                            if (cv_uc < 0) {
                                continue;
                            }
                            if (!is_O_equivalent_(uc, vc, rst)) {
                                if (cv_uc > static_cast<int>(rst.size())) {
                                    throw std::runtime_error("make_rst_consistent(): Unexpected cv which is out of bound of rst was encountered.");
                                }
                                auto &uc_table = rst.get_tables()[cv_uc];
                                for (auto &clabel : uc_table.get_col_labels()) {  // FIXME Small opti here (on at())
                                    if (uc_table.at(uc, clabel) != uc_table.at(vc, clabel)) {
                                        auto new_s = c + clabel;
                                        rst.add_row_using_query(new_s, cv_uc, teacher_, "make_consistent");
                                    }
                                }

                                return false;
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    /**
     * Make the RST closed
     * The algorithm is the following (TODO)
     * @param rst The RST
     * @return true if the RST was already closed, false if a change was made
     */
    bool learner::make_rst_closed(RST &rst) {
        for (size_t i = 0; i < rst.size(); ++i) {
            auto &table = rst.get_tables()[i];
            for (const std::string& u: table.get_row_labels()) {
                for (auto c : alphabet_.symbols()) {

                    int val = get_cv(c);
                    if ((val == -1 and i == 0) or (val == 1 and i == rst.size() - 1)) {
                        continue;
                    }

                    std::string uc = u + c;
                    auto uc_O = get_congruence_set_(uc, rst);
                    int cv_uc = static_cast<int>(i) + val;

                    auto &uc_table = rst.get_tables()[cv_uc];
                    if (std::find(uc_table.get_row_labels().begin(), uc_table.get_row_labels().end(), uc) != uc_table.get_row_labels().end()) {
                        continue;
                    }

                    // Checking if there is words in common in iXc_table (rows) and uc_O
                    bool empty_inter = true;
                    for (const auto& congruence_word : uc_O) {
                        for (const auto& clabel : uc_table.get_row_labels()) {
                            if (congruence_word == clabel) {
                                empty_inter = false;
                                break;
                            }
                        }
                        if (!empty_inter)
                            break;
                    }

                    if (empty_inter) {
                        std::cout << "Adding '" << uc << "'.\n";
                        rst.add_row_using_query(uc, cv_uc, teacher_, "make_closed");
                        return false;
                    }
                }
            }
        }

        return true;
    }

    /**
     * Lean a V1CA from the given alphabet and teacher
     * @param verbose Set to true for debug printing
     * @return The obtained V1CA
     */
    V1CA learner::learn_V1CA(bool verbose)
    {
        mode_ = learner_mode::V1CA;
        if (!as_visibly_alphabet_)
            throw std::invalid_argument("Learning a V1CA requires a visibly type of alphabet.");

        // Initialising rst with "" and "" as only labels for rows and columns
        auto rst = RST(teacher_);
        std::shared_ptr<V1CA> res = nullptr;

        // Looping until V1CA is accepted by teacher
        auto v1ca_correct = false;
        while (!v1ca_correct) {

            auto is_consistent = false;
            auto is_closed = false;

            // Looping while RST is not closed and consistent
            while (!is_consistent or !is_closed) {
                is_consistent = make_rst_consistent(rst);
                is_closed = make_rst_closed(rst);
                // Some debug print
                if (verbose) {
                    std::cout << "RST after trying to make it closed/consistent:\n";
                    std::cout << rst;
                    std::cout << "Consistent: " << is_consistent << ", closed: " << is_closed << std::endl;
                }
            }

            // Removing duplicates inside RST (to avoid state duplication)
            RST rst_no_dup = rst.remove_duplicate_rows();
            // Creating behaviour graph
            auto bg = behaviour_graph(rst_no_dup, *as_visibly_alphabet_, teacher_, alphabet_);

            // Testing partial equivalence on behaviour graph
            auto partial_eq = teacher_.partial_equivalence_query(bg, "behaviour_graph");
            if (!partial_eq) {
                // Making V1CA by (maybe) finding a periodic subgraph
                res = bg.to_v1ca(rst_no_dup, *as_visibly_alphabet_, verbose);
                // Testing V1CA equivalence
                auto eq = teacher_.equivalence_query(*res, "v1ca");
                if (!eq)
                    v1ca_correct = true;
                else
                    rst.add_counter_example(*eq, teacher_, *as_visibly_alphabet_);
            } else {
                rst.add_counter_example(*partial_eq, teacher_, *as_visibly_alphabet_);
            }
        }

        return *res;
    }

    R1CA learner::learn_R1CA(bool verbose) {
        if (!as_basic_alphabet_)
            throw std::invalid_argument("Learning a R1CA requires a basic type of alphabet.");

        mode_ = learner_mode::R1CA;
        // Initialising rst with "" and "" as only labels for rows and columns
        auto rst = RST(teacher_);
        std::shared_ptr<R1CA> res = nullptr;

        // Looping until V1CA is accepted by teacher
        auto v1ca_correct = false;
        while (!v1ca_correct) {

            auto is_consistent = false;
            auto is_closed = false;

            // Looping while RST is not closed and consistent
            while (!is_consistent or !is_closed) {
                is_consistent = make_rst_consistent(rst);
                is_closed = make_rst_closed(rst);
                // Some debug print
                if (verbose) {
                    std::cout << "RST after trying to make it closed/consistent:\n";
                    std::cout << rst;
                    std::cout << "Consistent: " << is_consistent << ", closed: " << is_closed << std::endl;
                }
            }

            // Removing duplicates inside RST (to avoid state duplication)
            RST rst_no_dup = rst.remove_duplicate_rows();
            // Creating behaviour graph
            auto bg = behaviour_graph(rst_no_dup, *as_automaton_teacher_, teacher_, alphabet_);

            // Testing partial equivalence on behaviour graph
            auto partial_eq = teacher_.partial_equivalence_query(bg, "behaviour_graph");
            if (!partial_eq) {
                // Making V1CA by (maybe) finding a periodic subgraph
                res = bg.to_r1ca(rst_no_dup, *as_basic_alphabet_, verbose);
                // Testing V1CA equivalence
                auto eq = teacher_.equivalence_query(*res, "v1ca");
                if (!eq)
                    v1ca_correct = true;
                else
                    rst.add_counter_example(*eq, teacher_, *as_automaton_teacher_);
            } else {
                rst.add_counter_example(*partial_eq, teacher_, *as_automaton_teacher_);
            }
        }

        return *res;
    }


    int learner::get_cv(const std::string &word) {
        if (mode_ == learner_mode::V1CA)
            return as_visibly_alphabet_->get_cv(word);
        else if (mode_ == learner_mode::R1CA)
            return dynamic_cast<automaton_teacher&>(teacher_).count_query(word);

        throw std::runtime_error("Unknown learner_type while processing cv");
    }

    int learner::get_cv(char symbol) {
        if (mode_ == learner_mode::V1CA) {
            return as_visibly_alphabet_->get_cv(symbol);
        } else if (mode_ == learner_mode::R1CA) {
            auto symbol_as_str = std::string() + symbol;
            return dynamic_cast<automaton_teacher&>(teacher_).count_query(symbol_as_str);
        }

        throw std::runtime_error("Unknown learner_type while processing cv");
    }

    std::set<std::string> learner::get_congruence_set_(const std::string &word, RST &rst) {

        if (mode_ == learner_mode::V1CA) {
            return get_congruence_set(word, rst, *as_visibly_alphabet_, teacher_);
        } else if (mode_ == learner_mode::R1CA) {
            return get_congruence_set(word, rst, dynamic_cast<automaton_teacher&>(teacher_), teacher_);
        }

        throw std::runtime_error("Unknown learner_type while processing congruence set");
    }

    bool learner::is_O_equivalent_(const std::string &word1, const std::string &word2, RST &rst) {

        if (mode_ == learner_mode::V1CA) {
            return is_O_equivalent(word1, word2, rst, *as_visibly_alphabet_, teacher_);
        } else if (mode_ == learner_mode::R1CA) {
            return is_O_equivalent(word1, word2, rst, dynamic_cast<automaton_teacher&>(teacher_), teacher_);
        }

        throw std::runtime_error("Unknown learner_type while processing O_equivalent");
    }

}

