#pragma once

namespace utils {

    // Used for pair inside set
    // Comparable pair
    template<class T1, class T2>
    struct pair_comp {

        pair_comp(T1 e1, T2 e2) : first(e1), second(e2) {}

        bool operator==(const pair_comp<T1, T2> &p1) const {
            return p1.first == first and p1.second == second;
        }

        bool operator<(const pair_comp<T1, T2> &p1) const {
            return (first + second * 65536) > (p1.first + p1.second * 65536);
        }

        T1 first;
        T2 second;
    };

    template<class T1, class T2>
    inline pair_comp<T1, T2> make_pair_comp(T1 e1, T2 e2) {
        return pair_comp(e1, e2);
    }

    template <class T1, class T2, class T3>
    struct triple_comp {

        triple_comp(T1 e1, T2 e2, T3 e3) : first(e1), second(e2), third{e3} {}

        bool operator==(const triple_comp<T1, T2, T3> &t1) const {
            return t1.first == first and t1.second == second and t1.third == third;
        }

        bool operator<(const triple_comp<T1, T2, T3> &t1) const {
            if (first != t1.first)
                return first < t1.first;
            if (second != t1.second)
                return second < t1.second;

            return third < t1.third;
        }

        T1 first;
        T2 second;
        T3 third;
    };

    template<class T1, class T2, class T3>
    inline triple_comp<T1, T2, T3> make_triple_comp(T1 e1, T2 e2, T3 e3) {
        return triple_comp(e1, e2, e3);
    }
}
