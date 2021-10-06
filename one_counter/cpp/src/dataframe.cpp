#include "dataframe.h"

#include <iostream>

namespace active_learning {

    /**
     * Add a row to the table. The row is filled with false values
     * @param name The name of the new row
     */
    void RST::RST_table::add_row(const std::string &name) {
        row_labels_.emplace_back(name);

        data_.emplace_back();
        (data_.end() - 1)->resize(col_labels_.size(), false);
    }

    /**
     * Add a new column to the table. The column is filled with false values
     * @param name The name of the new column
     */
    void RST::RST_table::add_col(const std::string &name) {
        col_labels_.emplace_back(name);
        for (auto &row : data_) {
            row.emplace_back(false);
        }
    }

    /**
     * Add a new row to the table. The values of the row are deduced using the teacher's
     * membership query
     * @param name The name of the new row
     * @param teacher The teacher used to fill the row
     */
    void RST::RST_table::add_row_using_query(const std::string &name, teacher &teacher) {
        add_row(name);
        auto &row = *(data_.end() - 1);
        auto &label = *(row_labels_.end() - 1);
        for (size_t i = 0; i < col_labels_.size(); ++i) {
            row[i] = teacher.membership_query(label + col_labels_[i]);
        }
    }

    /**
     * Add a new column to the table. The values of the column are deduced using the teacher's
     * membership query
     * @param name The name of the new column
     * @param teacher The teacher used to fill the column
     */
    void RST::RST_table::add_col_using_query(const std::string &name, teacher &teacher) {
        add_col(name);
        for (size_t i = 0; i < row_labels_.size(); ++i) {
            auto &row = data_[i];
            auto &label = row_labels_[i];
            *(row.end() - 1) = teacher.membership_query(label + name);
        }
    }

    /**
     * Class getter
     * @return The const vector of columns labels
     */
    const std::vector<std::string> &RST::RST_table::get_col_labels() const {
        return col_labels_;
    }

    /**
     * Class getter
     * @return The const vector of row labels
     */
    const std::vector<std::string> &RST::RST_table::get_row_labels() const {
        return row_labels_;
    }

    /**
     * Class getter
     * @return The table of boolean values without the labels
     */
    std::vector<std::vector<bool>> &RST::RST_table::get_data() {
        return data_;
    }

    /**
     * Class getter
     * @return The table of boolean values without the labels as a cont val
     */
    const std::vector<std::vector<bool>> &RST::RST_table::get_cdata() const {
        return data_;
    }

    /**
     * Get the bool value of the table at the index [row, col]
     * @param row_index The integer index of row
     * @param col_index The integer index of column
     * @return The value in the table at index [row, col]
     */
    bool RST::RST_table::at(size_t row_index, size_t col_index) const {
        return data_[row_index][col_index];
    }

    /**
     * Get the bool value of the table at the index [row, col]
     * @param row_index The string label of the row
     * @param col_index The string label of the column
     * @return The value in the table at index [row, col]
     */
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

    /**
     * Get the bool value of the table at the index [row, col]
     * @param row_index The integer index of row
     * @param col_index The string label of the column
     * @return The value in the table at index [row, col]
     */
    bool RST::RST_table::at(size_t row_index, const std::string &col) const {
        // Finding col index
        auto col_pos = std::find(col_labels_.begin(), col_labels_.end(), col);
        if (col_pos == col_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find column '" + col + "' in table");
        }
        size_t col_index = col_pos - col_labels_.begin();

        return at(row_index, col_index);
    }

    /**
     * Get the bool value of the table at the index [row, col]
     * @param row_index The string label of the row
     * @param col_index The integer index of column
     * @return The value in the table at index [row, col]
     */
    bool RST::RST_table::at(const std::string &row, size_t col_index) const {
        // Finding row_index
        auto row_pos = std::find(row_labels_.begin(), row_labels_.end(), row);
        if (row_pos == row_labels_.end()) {
            throw std::invalid_argument("RST::RST_Table::at(): Could not find row '" + row + "' in table");
        }
        size_t row_index = row_pos - row_labels_.begin();

        return at(row_index, col_index);
    }

    /**
     * Class getter
     * @return The mutable vector of columns labels
     */
    std::vector<std::string> &RST::RST_table::get_mutable_row_labels() {
        return row_labels_;
    }

    /**
     * Add new tables to the list of tables until there is as many tables as cv
     * @param cv The number of tables required (max counter value)
     */
    void RST::expand_RST(int cv) {
        while (cv >= static_cast<int>(tables_.size())) {
            tables_.emplace_back();
        }
    }

    /**
     * Detect the rows that have the same boolean values for each of column,
     * and only leave the first occurrence of the row, removing the other rows
     * @return A copy of the RST with possibly rows removed. The original RST is not changed.
     */
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

    /**
     * Add a new row at the right table. Row is filled with false values
     * @param name The name of the new row
     * @param cv The counter value (index) of the table. cv should be cv(name),
     * but this is not checked for performance reasons
     */
    void RST::add_row(const std::string &name, int cv) {
        expand_RST(cv);
        tables_[cv].add_row(name);
    }

    /**
     * Add a new row at the right table, and fills it using the teacher membership query
     * @param name The name of the new row
     * @param cv The counter value (index) of the table. cv should be cv(name),
     * but this is not checked for performance reasons
     * @param teacher The teacher used for the membership query
     */
    void RST::add_row_using_query(const std::string &name, int cv, teacher &teacher) {
        expand_RST(cv);
        tables_[cv].add_row_using_query(name, teacher);
    }

    /**
     * Add a new column at the right table, and fills it with false values
     * @param name The name of the new column
     * @param cv The counter value (index) of the table. cv should be cv(name),
     * but this is not checked for performance reasons
     */
    void RST::add_col(const std::string &name, int cv) {
        expand_RST(cv);
        tables_[cv].add_col(name);
    }

    /**
     * Add a new column at the right table, and fills it using the teacher membership query
     * @param name The name of the new column
     * @param cv The counter value (index) of the table. cv should be cv(name),
     * but this is not checked for performance reasons
     * @param teacher The teacher used for the membership query
     */
    void RST::add_col_using_query(const std::string &name, int cv, teacher &teacher) {
        expand_RST(cv);
        tables_[cv].add_col_using_query(name, teacher);
    }

    /**
     * Create a new RST, and fills the value at ["", ""] using the teacher membership query
     * @param teacher The teacher used for the membership query
     */
    RST::RST(teacher &teacher) {
        tables_.emplace_back();
        tables_[0].add_col("");
        add_row_using_query("", 0, teacher, "rst init");
    }

    /**
     * Class getter
     * @return The mutable vector of tables
     */
    std::vector<RST::RST_table> &RST::get_tables() {
        return tables_;
    }

    /**
     * Class getter
     * @return The const vector of tables
     */
    const std::vector<RST::RST_table> &RST::get_ctables() const {
        return tables_;
    }

    /**
     * Compares the boolean values of two rows of a RST from the same table.
     * @param word1 The label of the first row
     * @param word2 The label of the second row
     * @param cv The counter value of the two words. cv(word1) == cv(word2) == cv must be true
     * @return true if the two row are duplicated ones (identical). false otherwise
     */
    bool RST::compare_rows(const std::string &word1, const std::string &word2, int cv) const {
        if (cv >= static_cast<int>(tables_.size()) or cv < 0) {
            throw std::runtime_error("compare_row(): CV out of bound for this RST.");
        }
        RST_table table = tables_[cv];

        auto &row_labels = table.get_row_labels();
        // Finding 1st word
        auto word1_pos = std::find(row_labels.begin(), row_labels.end(), word1);
        if (word1_pos == row_labels.end()) {
            throw std::invalid_argument("compare_rows(): word1 is not present at given cv in the table.");
        }
        auto word1_index = word1_pos - row_labels.begin();

        // Finding 2nd word
        auto word2_pos = std::find(row_labels.begin(), row_labels.end(), word2);
        if (word2_pos == row_labels.end()) {
            throw std::invalid_argument("compare_rows(): word2 is not present at given cv in the table.");
        }
        auto word2_index = word2_pos - row_labels.begin();

        auto &word1_row = table.get_data()[word1_index];
        auto &word2_row = table.get_data()[word2_index];

        return word1_row == word2_row;
    }

    /**
     * Get the size of the RST
     * @return The number of table in the RST
     */
    size_t RST::size() const {
        return tables_.size();
    }

    /**
     * Add a counter example to the RST.
     * For each prefix of the counter example (including the counter example itself), adds it as a new row label
     * For each suffix of the counter example (including the counter example itself), adds it as a new column label
     * Rows are filled using a teacher membership query.
     * @param ce The counter example word
     * @param teacher The teacher used for the membership query
     * @param alphabet The target language alphabet
     */
    void RST::add_counter_example(const std::string &ce, teacher &teacher, visibly_alphabet_t &alphabet) {
        for (const auto &word : get_all_prefixes(ce)) {

            int cv = get_cv(word, alphabet);
            expand_RST(cv);
            auto &table = tables_[cv];

            auto found = std::find(table.get_row_labels().begin(), table.get_row_labels().end(), word);
            if (found == table.get_row_labels().end()) {
                add_row_using_query(word, cv, teacher, "ce row");
            }

            auto suff = ce.substr(word.size());
            auto &table_cols = tables_[cv].get_col_labels();
            if (std::find(table_cols.begin(), table_cols.end(), suff) == table_cols.end()) {
                add_col_using_query(suff, cv, teacher, "ce col");
            }
        }
    }

    /**
     * Debug function, does the same thing, context is unused
     * Remove comment for debug print
     */
    void RST::add_col_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context) {
        (void) context;
        //std::cout << '(' << context << ") ";
        add_col_using_query(name, cv, teacher);
    }

    /**
     * Debug function, does the same thing, context is unused
     * Remove comment for debug print
     */
    void RST::add_row_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context) {
        // std::cout << '(' << context << ") ";
        (void) context;
        add_row_using_query(name, cv, teacher);
    }

    /**
     * Add a column to the RST similarly to "add_col_using_query",
     * but the column is not added if the label already exist
     * @param name The name of the new column
     * @param cv The counter value of the name, cv(name) == cv must be true
     * @param teacher The teacher used for the membership query
     * @param context Debug print
     */
    void RST::add_col_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                                 const std::string &context) {
        auto &table = tables_[cv];
        auto &col_labels = table.get_col_labels();
        if (std::find(col_labels.begin(), col_labels.end(), name) != col_labels.end()) {
            return;
        }

        add_col_using_query(name, cv, teacher, context);
    }

    /**
     * Add a row to the RST similarly to "add_col_using_query",
     * but the row is not added if the label already exist
     * @param name The name of the new row
     * @param cv The counter value of the name, cv(name) == cv must be true
     * @param teacher The teacher used for the membership query
     * @param context Debug print
     */
    void RST::add_row_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                                 const std::string &context) {

        auto &table = tables_[cv];
        auto &row_labels = table.get_row_labels();
        if (std::find(row_labels.begin(), row_labels.end(), name) != row_labels.end()) {
            return;
        }

        add_row_using_query(name, cv, teacher, context);
    }

    /**
     * Print the table onto the stream
     * @param out The stream
     * @param table The table to be printed
     * @return The input stream
     */
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

    /**
     * Print the RST onto the stream
     * @param out The stream
     * @param table The table to be printed
     * @return The input stream
     */
    std::ostream &operator<<(std::ostream &out, const RST &rst) {
        for (size_t i = 0; i < rst.get_ctables().size(); ++i) {
            const auto &table = rst.get_ctables()[i];
            out << "Table cv = " << i << "\n";
            out << table;
        }

        return out;
    }
}
