#include <iostream>
#include "V1CA_learner.h"
#include "dataframe.h"

#include <iostream>

namespace active_learning {

    V1CA_learner::V1CA_learner(teacher &teacher, alphabet_t &alphabet) : teacher_(teacher), alphabet_(alphabet) {}

    bool V1CA_learner::make_rst_consistent(RST &rst) {
        for (auto &table : rst.get_tables()) {
            for (auto u_i = 0u; u_i < table.get_row_labels().size(); ++u_i) {
                const std::string &u = table.get_row_labels()[u_i];
                for (auto v_i = u_i + 1; v_i < table.get_row_labels().size(); ++v_i) {
                    const std::string &v = table.get_row_labels()[v_i];
                    if (is_O_equivalent(u, v, rst, teacher_, alphabet_)) {
                        for (auto &symbol : alphabet_) {
                            char c = symbol.first;
                            std::string uc = u + c;
                            std::string vc = v + c;
                            auto cv_uc = get_cv(uc, alphabet_);
                            if (cv_uc < 0) {
                                continue;
                            }
                            if (!is_O_equivalent(uc, vc, rst, teacher_, alphabet_)) {
                                if (cv_uc > static_cast<int>(rst.size())) {
                                    throw new std::runtime_error("make_rst_consistent(): Unexpected cv which is out of bound of rst was encountered.");
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

    bool V1CA_learner::make_rst_closed(RST &rst) {
        for (size_t i = 0; i < rst.size(); ++i) {
            auto &table = rst.get_tables()[i];
            for (const std::string& u: table.get_row_labels()) {
                for (auto symbol : alphabet_) {

                    char c = symbol.first;
                    int val = symbol.second;
                    if ((val == -1 and i == 0) or (val == 1 and i == rst.size() - 1)) {
                        continue;
                    }

                    std::string uc = u + c;
                    auto uc_O = get_congruence_set(uc, rst, teacher_, alphabet_);
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

    V1CA V1CA_learner::learn_V1CA(bool verbose)
    {
        // Initialising rst with "" and "" as only labels for rows and columns
        auto rst = RST(teacher_);
        auto res = V1CA();

        // Looping until V1CA is accepted by teacher
        auto v1ca_correct = false;
        while (!v1ca_correct) {

            auto is_consistent = false;
            auto is_closed = false;

            // Looping until RST is not closed and consistent
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
            auto rst_no_dup = rst.remove_duplicate_rows();
            // Creating behaviour graph
            auto behaviour_graph = V1CA_builder::build_behaviour_graph_from_RST(rst_no_dup, alphabet_, teacher_);
            // Testing partial equivalence on behaviour graph
            auto partial_eq = teacher_.partial_equivalence_query(behaviour_graph, "behaviour_graph");
            if (!partial_eq) {
                // Making V1CA by (maybe) finding a periodic subgraph
                res = V1CA_builder::behaviour_graph_to_V1CA(behaviour_graph, rst_no_dup, alphabet_, verbose);  // inplace
                // Testing V1CA equivalence
                auto eq = teacher_.equivalence_query(res, "v1ca");
                if (!eq)
                    v1ca_correct = true;
                else
                    rst.add_counter_example(*eq, teacher_, alphabet_);
            } else {
                rst.add_counter_example(*partial_eq, teacher_, alphabet_);
            }
        }

        return res;
    }

}

