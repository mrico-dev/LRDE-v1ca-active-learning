#include "dataframe.h"

#include <iostream>

namespace active_learning {

    void RST::RST_table::add_row(const std::string &name) {
        row_labels_.emplace_back(name);

        data_.emplace_back();
        (data_.end() - 1)->resize(col_labels_.size(), false);
    }

    void RST::RST_table::add_col(const std::string &name) {
        col_labels_.emplace_back(name);
        for (auto &row : data_) {
            row.emplace_back(false);
        }
    }

    void RST::RST_table::add_row_using_query(const std::string &name, teacher &teacher) {
        add_row(name);
        auto &row = *(data_.end() - 1);
        auto &label = *(row_labels_.end() - 1);
        for (size_t i = 0; i < col_labels_.size(); ++i) {
            row[i] = teacher.belong_query(label + col_labels_[i]);
        }
    }

    void RST::RST_table::add_col_using_query(const std::string &name, teacher &teacher) {
        add_col(name);
        for (size_t i = 0; i < row_labels_.size(); ++i) {
            auto &row = data_[i];
            auto &label = row_labels_[i];
            *(row.end() - 1) = teacher.belong_query(label + name);
        }
    }

    const std::vector<std::string> &RST::RST_table::get_col_labels() const {
        return col_labels_;
    }

    const std::vector<std::string> &RST::RST_table::get_row_labels() const {
        return row_labels_;
    }

    std::vector<std::vector<bool>> &RST::RST_table::get_data() {
        return data_;
    }

    const std::vector<std::vector<bool>> &RST::RST_table::get_cdata() const {
        return data_;
    }

    bool RST::RST_table::at(size_t row_index, size_t col_index) const {
        return data_[col_index][row_index];
    }

    bool RST::RST_table::at(const std::string &row, const std::string &col) const {
        // Finding row_index
        auto row_pos = std::find(row_labels_.begin(), row_labels_.end(), row);
        if (row_pos == row_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find row '" + row + "' in table");
        }
        size_t row_index = row_pos - row_labels_.begin();
        // Finding col index
        auto col_pos = std::find(col_labels_.begin(), col_labels_.end(), col);
        if (col_pos == col_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find column '" + col + "' in table");
        }
        size_t col_index = col_pos - col_labels_.begin();

        return at(row_index, col_index);
    }

    bool RST::RST_table::at(size_t row_index, const std::string &col) const {
        // Finding col index
        auto col_pos = std::find(col_labels_.begin(), col_labels_.end(), col);
        if (col_pos == col_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find column '" + col + "' in table");
        }
        size_t col_index = col_pos - col_labels_.begin();

        return at(row_index, col_index);
    }

    bool RST::RST_table::at(const std::string &row, size_t col_index) const {
        // Finding row_index
        auto row_pos = std::find(row_labels_.begin(), row_labels_.end(), row);
        if (row_pos == row_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find row '" + row + "' in table");
        }
        size_t row_index = row_pos - row_labels_.begin();

        return at(row_index, col_index);
    }

    std::vector<std::string> &RST::RST_table::get_mutable_row_labels() {
        return row_labels_;
    }

    void RST::expand_RST(int cv) {
        while (cv >= static_cast<int>(tables_.size())) {
            tables_.emplace_back();
        }
    }

    RST RST::remove_duplicate_rows() const {
        // Creating copy (not in place)
        RST res = RST(*this);

        for (auto &table : res.get_tables()) {
            for (auto row_i = 0u; row_i < table.get_data().size(); ++row_i) {
                const auto &row = table.get_data()[row_i];
                for (auto other_row_i = row_i + 1; other_row_i < table.get_data().size(); ++other_row_i) {
                    if (table.get_data()[other_row_i] == row) {
                        table.get_data().erase(table.get_data().begin() + other_row_i);
                        table.get_mutable_row_labels().erase(table.get_row_labels().cbegin() + other_row_i);
                    }
                }
            }
        }

        return res;
    }

    void RST::add_row(const std::string &name, int cv) {
        expand_RST(cv);
        tables_[cv].add_row(name);
    }

    void RST::add_row_using_query(const std::string &name, int cv, teacher &teacher) {
        expand_RST(cv);
        tables_[cv].add_row_using_query(name, teacher);
    }

    void RST::add_col(const std::string &name, int cv) {
        expand_RST(cv);
        tables_[cv].add_col(name);
    }

    void RST::add_col_using_query(const std::string &name, int cv, teacher &teacher) {
        expand_RST(cv);
        tables_[cv].add_col_using_query(name, teacher);
    }

    RST::RST(teacher &teacher) {
        tables_.emplace_back();
        tables_[0].add_col("");
        add_row_using_query("", 0, teacher, "rst init");
    }

    std::vector<RST::RST_table> &RST::get_tables() {
        return tables_;
    }

    const std::vector<RST::RST_table> &RST::get_ctables() const {
        return tables_;
    }

    bool RST::compare_rows(const std::string &word1, const std::string &word2, int cv) const {
        if (cv >= static_cast<int>(tables_.size()) or cv < 0) {
            throw std::runtime_error("compare_row(): CV out of bound for this RST.");
        }
        RST_table table = tables_[cv];

        auto &row_labels = table.get_row_labels();
        // Finding 1st word
        auto word1_pos = std::find(row_labels.begin(), row_labels.end(), word1);
        if (word1_pos == row_labels.end()) {
            throw new std::invalid_argument("compare_rows(): word1 is not present at given cv in the table.");
        }
        auto word1_index = word1_pos - row_labels.begin();
        // Finding 2nd word
        auto word2_pos = std::find(row_labels.begin(), row_labels.end(), word2);
        if (word2_pos == row_labels.end()) {
            throw new std::invalid_argument("compare_rows(): word2 is not present at given cv in the table.");
        }
        auto word2_index = word2_pos - row_labels.begin();

        auto &word1_row = table.get_data()[word1_index];
        auto &word2_row = table.get_data()[word2_index];

        return word1_row == word2_row;
    }

    size_t RST::size() const {
        return tables_.size();
    }

    void RST::add_counter_example(const std::string &ce, teacher &teacher, alphabet_t &alphabet) {
        for (const auto& word : get_all_prefixes(ce)) {

            int cv = get_cv(word, alphabet);
            expand_RST(cv);
            auto &table = tables_[cv];

            auto found = std::find(table.get_row_labels().begin(), table.get_row_labels().end(), word);
            if (found == table.get_row_labels().end()) {
                add_row_using_query(word, cv, teacher, "ce row");
            }

            auto suff = ce.substr(word.size());
            auto suff_cv = -get_cv(suff, alphabet); // should be the same as cv but just in case FIXME Remove later
            auto &table_cols = tables_[suff_cv].get_col_labels();
            if (std::find(table_cols.begin(), table_cols.end(), suff) == table_cols.end()) {
                add_col_using_query(suff, suff_cv, teacher, "ce col");
            }
        }
    }

    void RST::add_col_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context) {
        (void) context;
        //std::cout << '(' << context << ") ";
        add_col_using_query(name, cv, teacher);
    }

    void RST::add_row_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context) {
        //std::cout << '(' << context << ") ";
        (void) context;
        add_row_using_query(name, cv, teacher);
    }

    void RST::add_col_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                                 const std::string &context) {
        auto &table = tables_[cv];
        auto &col_labels = table.get_col_labels();
        if (std::find(col_labels.begin(), col_labels.end(), name) != col_labels.end()) {
            return;
        }

        add_col_using_query(name, cv, teacher, context);
    }

    void RST::add_row_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                         const std::string &context) {

        auto &table = tables_[cv];
        auto &row_labels = table.get_row_labels();
        if (std::find(row_labels.begin(), row_labels.end(), name) != row_labels.end()) {
            return;
        }

        add_row_using_query(name, cv, teacher, context);
    }

    std::ostream &operator<<(std::ostream &out, const RST::RST_table &table) {
        auto padding = 10u;

        // Top line
        for (auto i = 0u; i < padding; ++i) {
            out << ' ';
        }
        for (auto &clabel : table.get_col_labels()) {
            out << clabel;
            for (auto i = 0u; i < padding - clabel.size(); ++i) {
                out << ' ';
            }
        }
        out << '\n';

        // Other lines
        for (size_t i = 0; i < table.get_row_labels().size(); ++i) {
            out << table.get_row_labels()[i];
            const auto &row = table.get_cdata()[i];
            for (auto j = 0u; j < padding - table.get_row_labels()[i].size(); ++j) {
                out << ' ';
            }

            for (const auto &e : row) {
                if (e) {
                    out << "True      ";
                } else {
                    out << "False     ";
                }
            }
            out << '\n';
        }

        return out;
    }

    std::ostream &operator<<(std::ostream &out, const RST &rst) {
        for (size_t i = 0; i < rst.get_ctables().size(); ++i) {
            const auto &table = rst.get_ctables()[i];
            out << "Table cv = " << i << "\n";
            out << table;
        }

        return out;
    }
}
