#pragma once

#include <vector>
#include <string>
#include <map>

namespace active_learning {
    class teacher;

    using visibly_alphabet_t = std::map<char, int>;
}

#include "language.h"
#include "teacher.h"

namespace active_learning {

    class RST {

    public:

        class RST_table {

        public:

            RST_table() = default;

            void add_row(const std::string &name);

            void add_row_using_query(const std::string &name, teacher &teacher);

            void add_col(const std::string &name);

            void add_col_using_query(const std::string &name, teacher &teacher);


            const std::vector<std::string> &get_col_labels() const;

            const std::vector<std::string> &get_row_labels() const;

            std::vector<std::string> &get_mutable_row_labels();

            std::vector<std::vector<bool>> &get_data();

            const std::vector<std::vector<bool>> &get_cdata() const;

            bool at(const std::string &row, const std::string &col) const;

            bool at(const std::string &row, size_t col_index) const;

            bool at(size_t row_index, const std::string &col) const;

            inline bool at(size_t row_index, size_t col_index) const;

        private:
            std::vector<std::string> col_labels_;
            std::vector<std::string> row_labels_;
            std::vector<std::vector<bool>> data_;
        };

    private:
        void expand_RST(int cv);

    public:
        explicit RST(teacher &teacher);

        RST remove_duplicate_rows() const;

        void add_row(const std::string &name, int cv);

        void add_col(const std::string &name, int cv);

        void add_row_using_query(const std::string &name, int cv, teacher &teacher);

        void add_col_using_query(const std::string &name, int cv, teacher &teacher);

        void add_row_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context);

        void add_col_using_query(const std::string &name, int cv, teacher &teacher, const std::string &context);

        void add_row_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                                const std::string &context);

        void add_col_using_query_if_not_present(const std::string &name, int cv, teacher &teacher,
                                                const std::string &context);

        void add_counter_example(const std::string &ce, teacher &teacher, visibly_alphabet_t &alphabet);

        bool compare_rows(const std::string &word1, const std::string &word2, int cv) const;

        size_t size() const;

        std::vector<RST_table> &get_tables();

        const std::vector<RST_table> &get_ctables() const;

    private:
        std::vector<RST_table> tables_;
    };

    std::ostream &operator<<(std::ostream &out, const RST &rst);

    std::ostream &operator<<(std::ostream &out, const RST::RST_table &table);
}

//V1C2AL_DATAFRAME_H
