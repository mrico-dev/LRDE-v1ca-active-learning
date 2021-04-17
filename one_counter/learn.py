#!/bin/python3

import pandas as pd
import numpy as np
import re
import copy

import tools.one_counter_tools as oc
import tools.display_automaton as display

class Teacher:

    prev = {}

    def __init__(self, regex: str or None = None, func: callable or None = None):
        self.regex = regex
        self.func = func


    def accepts_word(self, word: str) -> bool:
        """
        Tell whether a word is accepted or not
        """
        if self.regex is not None:
            res = re.match("^" + regex + "$", word) is not None
            if word not in self.prev.keys():
                self.prev[word] = res
            return res

        if self.func is not None:
            res = self.func(word)
            if word not in self.prev.keys():
                self.prev[word] = res
            return bool(res)

        res = ""

        if word in self.prev:
            #print("We already know '" + word + "', skipping...")
            return self.prev[word]

        while res != "Y" and res != "N":
            res = input("Is the word '" + word+ "' accepted? [y/n] ")
            res = res.upper()

        res = (res == "Y")
        self.prev[word] = res

        return res

    def is_language(self, automata, automata_type: str = '') -> str or True:
        """
        Tell wheter the language is accepted, if not gives a counter example
        """
        if automata_type == 'one_counter':
            display.export_one_counter(automata, "learning_one_counter")
        else:
            raise Exception("Unknown automata type")

        res = input("Please compare the two automata and give a counter example\
                (or OK if the automata is good): ")

        return True if res == "OK" else res


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


def get_all_prefixes(word: str):
    """
    Get all prefixes of a word
    """
    res = [""]
    pref = ""
    for c in word:
        pref += c
        res.append(pref)

    return res


def display_RST(RST) -> None:
    """
    Pretty print for RST
    """
    for i in range(len(RST)):
        print("LEVEL " + str(i) + ":")
        print(RST[i])


def get_cv(w: str, wmapper: dict) -> int:
    """
    Get the height of a word w
    """
    res = 0
    for i in range(len(w)):
        res += wmapper[w[i]]

    return res


def is_O_equivalent(RST, u: str, v: str, teacher: Teacher, wmapper: dict) -> bool:
    """
    Returns wether u ~O v
    u et v O-equivalent: cv(u) = cv(v) 
        et pout tout w dans S_i (i = cv(u)),
            T(uw) = T(vw)
    """
    cv_u = get_cv(u, wmapper)
    if cv_u != get_cv(v, wmapper):
        return False

    RST = copy.deepcopy(RST)
    extend_RST(RST, u, cv_u, teacher)
    extend_RST(RST, v, cv_u, teacher)
    RST_i = RST[cv_u]

    return bool(RST_i.loc[u].eq(RST_i.loc[v]).values.all())


def get_congruence_set(u: str, RST, alphabet, teacher):
    """
    Get the set [u]_O
    """
    cv_u = get_cv(u, alphabet[1])

    if cv_u > len(RST):
        raise Exception("What to do here?")

    res = set()
    for rst in RST:
        for v in list(rst.index):
            if is_O_equivalent(RST, u, v, teacher, wmapper=alphabet[1]):
                res.add(v)
            #for a in alphabet[0]:
            #    va = v + a
            #    if is_O_equivalent(RST, u, va, teacher, wmapper=alphabet[1]):
            #        res.add(va)

    return res


def init_RST(RST, teacher: Teacher):
    """
    Get the initial RST
    """
    RST0 = pd.DataFrame(index=[""], columns=[""], dtype=bool)
    RST0.at["", ""] = teacher.accepts_word("")

    return [RST0]


def fill_RST_row(RST_i, teacher: Teacher, row_name: str):
    """
    Fills a of RST[i] row using a teacher
    """
    for col in RST_i.columns:
        RST_i.at[row_name, col] = teacher.accepts_word(row_name + col)


def extend_RST(RST, w: str, cvw: int, teacher: Teacher):
    """
    Extends RST by adding new tables if necessary
    """
    if cvw < len(RST):
        if w in RST[cvw].index:
            return
        RST[cvw] = RST[cvw].append(pd.Series(name=w, dtype=bool))
        RST[cvw] = RST[cvw].astype(bool)
        fill_RST_row(RST[cvw], teacher, w)
        return

    while len(RST) <= cvw:
        new_RST_i = pd.DataFrame(dtype=bool)
        RST.append(new_RST_i)

    RST[cvw] = RST[cvw].append(pd.Series(name=w, dtype=bool))
    RST[cvw] = RST[cvw].astype(bool)
    fill_RST_row(RST[cvw], teacher, w)


def make_RST_consistent(RST, alphabet, teacher: Teacher):
    """
    Try to make the RST consistent
    """
    for rst in RST:
        for u in list(rst.index):
            for v in list(rst.index):
                if u != v and is_O_equivalent(RST, u, v, teacher, alphabet[1]):
                    for a in alphabet[0]:
                        ua = u + a
                        va = v + a
                        cv_ua = get_cv(ua, alphabet[1])
                        if cv_ua < 0:
                            continue
                        if not is_O_equivalent(RST, ua, va, teacher, alphabet[1]):
                            if (cv_ua > len(RST)):
                                raise Exception("What to do here?")

                            RST_i = RST[cv_ua]
                            # Making S in RST_i more complete
                            for s in list(RST_i.columns):
                                if RST_i.at[ua, s] != RST_i.at[va, s]:
                                    # Adding s to S
                                    new_s = a + s
                                    RST_i[s] = pd.Series(dtype=bool)
                                    RST[i] = RST[i].astype(bool)
                                    for r in list(RST_i.index):
                                        RST_i.at[r, s] = teacher.accepts_word(r + s)

                            return False

    return True


def make_RST_closed(RST, alphabet, teacher: Teacher):
    """
    Try to make the RST closed
    """
    wmapper = alphabet[1]
    for i in range(len(RST)):
        for u in RST[i].index:
            for a in alphabet[0]:

                # Special case (see closing def)
                if ((wmapper[a] == -1 and i == 0) or
                    (wmapper[a] == 1 and i == len(RST) - 1)):
                    continue

                ua    = u + a
                ua_O  = get_congruence_set(ua, RST, alphabet, teacher)
                iXa   = i + wmapper[a]

                # ???
                if ua in RST[iXa].index:
                    continue

                if iXa > len(RST):
                    raise Exception("What to do here?")

                R_iXa = set(RST[iXa].index)

                if ua_O.intersection(R_iXa) == set():
                    RST[iXa] = RST[iXa].append(pd.Series(name=ua, dtype=bool))
                    RST[iXa] = RST[iXa].astype(bool)
                    fill_RST_row(RST[iXa], teacher, ua)
                    return False

    return True


def add_counterexample_to_RST(ce, RST, teacher: Teacher, alphabet):
    """
    Add counter example to S (the rows) and fill them with teacher answer
    """
    teacher.prev[ce] = True
    for u in get_all_prefixes(ce):
        cvu = get_cv(u, alphabet[1])
        if cvu >= len(RST) or u not in list(RST[cvu].index):
            extend_RST(RST, u, cvu, teacher)

        suff = ce[len(u):]
        if suff not in RST[cvu].columns:
            RST[cvu][suff] = pd.Series(dtype=bool)
            RST[cvu] = RST[cvu].astype(bool)
            for r in list(RST[cvu].index):
                RST[cvu].at[r, suff] = teacher.accepts_word(r + suff)
            


def get_Ri_state_from_word(RST, w: str, teacher: Teacher, wmapper:dict) -> str:
    """
    Get the state [w]_O of word w inside RST_cvw which is RST_i where i = cv(w)
    return whether the state had to be created
    """
    cvw = get_cv(w, wmapper)
    RST_cvw = RST[cvw]

    if w in list(RST_cvw.index):
        return str(cvw) + "_" + w

    RST_cvw = RST_cvw.append(pd.Series(name=w, dtype=bool))
    RST_cvw = RST_cvw.astype(bool)
    fill_RST_row(RST_cvw, teacher, w)
    duplicate = RST_cvw[RST_cvw.duplicated(keep='last')]

    res = duplicate.index.values[0]
    RST_cvw.drop(index=w)

    return str(cvw) + "_" + res


def get_transitions(RST, alphabet, states):
    """
    Build transitions by evaluating u.a for every u in R_i and a in Sigma,
    and finding matching state [ua]_O
    Since RST is closed, there is always a matching state
    Taking state as argument so we dont have to process it twice
    """
    res = []
    for st in states:
        u = st[(st.index('_') + 1):]
        for a in alphabet[0]:
            ua = u + a
            ua_cv = get_cv(ua, alphabet[1])
            if ua_cv < 0 or ua_cv >= len(RST):
                continue

            # We are sur to find a dest since RST is closed and consistent
            dest = get_Ri_state_from_word(RST, ua, teacher, alphabet[1])
            res.append((st, a, dest))
            if dest not in states:
                raise RuntimeError("Found a destination " + str(dest) + " that was not a state. Not closed ?")

    return res


def delete_all_superior_levels(graph, cv):
    """
    Delete all levels greater than cv
    """
    for q in list(graph.states):
        cvq = int(q[:(q.index('_'))])
        if cvq > cv:
            graph.states.remove(q)
            for e in list(graph.edges):
                if e[0] == q or e[2] == q:
                    graph.edges.remove(e)


def get_sub_graph(level1, level2, graph):
    """
    Get a graph with only states of of level 1 to level 2
    Transitions from level2 are removed
    """
    # Remove all states < level 1 or > level2
    res = copy.deepcopy(graph)
    for q in list(res.states):
        cvq = int(q[:(q.index('_'))])
        if cvq > level2 or cvq < level1:
            res.remove_state(q)

    # Removing transition from level 2
    for e in list(res.edges):
        if int(e[0][:(e[0].index('_'))]) == level2:
            res.edges.remove(e)

    return res


def is_periodic(l1, l2, graph: oc.OneCounter, alphabet):
    """
    Checks wether there is a period at q1 and q2 of length k, with k = cv(q1) - cv(q2)
    """
    # Making sure there is no issue
    m = l1
    k = l2 - l1
    if k <= 0:
        raise RuntimeError("Cannot process period: l1>=l2 are the same: l1=" +str(l1)+ ", l2=" +str(l2))

    graph1 = get_sub_graph(l1, l2, graph)
    graph2 = get_sub_graph(l2, l2 + k, graph)

    return oc.is_isomorphic(graph1, graph2, l1, k)


def delete_all_superior_levels(graph, cv):
    """
    Delete all levels greater than cv
    """
    for q in list(graph.states):
        cvq = int(q[:(q.index('_'))])
        if cvq > cv:
            graph.states.remove(q)
            for e in list(graph.edges):
                if e[0] == q or e[2] == q:
                    graph.edges.remove(e)


def link_period(graph, couples, alphabet):
    """
    Link graph with period found between q1 and q2
    """
    for q1, q2 in couples:
        for src, a, dest in oc.get_edges_from_state(graph.edges, q1):
            graph.edges.append((q2, a, dest))


def detect_period_and_loop_back(graph: oc.OneCounter, RST, alphabet):
    """
    Do a parallel breadth first search to find periodic blocs
    """
    if len(RST) < 3:
        print("Graph is too small to find a periodic structure")
        return

    # m goes from 0 to len
    for m in range(len(RST) - 2):
        # k goes from (len - m) / 2 to 1
        for k in list(range(1, (len(RST) - m) // 2 + 1))[::-1]:

            # Checking if the 2 levels are isomorphic and if so the graph is periodic
            couples = is_periodic(m, m + k, graph, alphabet)
            if couples is not None:
                print("A periodic pattern between level " + str(m) + " and level '" + str(m + k) + "' was found.")
                delete_all_superior_levels(graph, m + k)
                link_period(graph, couple, alphabet)
                return 
        
    print("No periodic subgraph found. Showing behaviour graph as it is.")


def RST_to_one_counter(RST, alphabet, teacher):
    """
    Create a one_counter automata from the RST
    """
    no_dup_RST = []
    for rst in RST:
        no_dup_RST.append(rst.drop_duplicates(inplace=False))
    print("final_no_dup")
    display_RST(no_dup_RST)

    states = []
    for i in range(len(no_dup_RST)):
        for u in no_dup_RST[i].index:
            states.append(str(i) + "_" + u)
    final = ["0_" + u for u in no_dup_RST[0].index if no_dup_RST[0].at[u, ""]]
    init = "0_"
    # This part is a little (a lot) more tricky
    transitions = get_transitions(no_dup_RST, alphabet, states)
    
    print(states)
    print(final)
    print(init)
    print(transitions)

    behaviour = oc.OneCounter(states, [init], final, alphabet[0], alphabet[1], transitions)
    display.export_one_counter(behaviour, "behaviour_graph_unminimized")
    behaviour.remove_useless_states()
    display.export_one_counter(behaviour, "behaviour_graph")
    detect_period_and_loop_back(behaviour, no_dup_RST, alphabet)
    display.export_one_counter(behaviour, "one_counter_unminimized")
    behaviour.remove_useless_states()

    return behaviour


def learn_one_counter(alphabet, teacher: Teacher):
    """
    Initialize an empty stratified observation table O up to level t = 0
    repeat
        1. Learn BG L|t using membership and partial equivalence queries. -> main loop
        2. Identify all periodic descriptions β of BG L|t.                -> ??
        3. Construct a conjecture VCA Aβ for each periodic description.   -> building one-counter
        4. Conduct equivalence queries on the conjectures.                -> ask teacher 
        5. If a VCA accepting L is found, then stop and output this VCA.
            Otherwise choose one counter-example and add it to O. Thereby t increases. -> using counter example
    until a VCA recognizing L is found.
    """
    # RST Table (using panda)
    RST = init_RST(alphabet, teacher)

    is_language = False
    while not is_language:

        is_consistent = False
        is_closed = False

        while not is_consistent or not is_closed:
            display_RST(RST)
            is_consistent = make_RST_consistent(RST, alphabet, teacher)
            is_closed = make_RST_closed(RST, alphabet, teacher)
            print("is_closed: " + str(is_closed) + ", is_consistent:" + str(is_consistent))

        M = RST_to_one_counter(RST, alphabet, teacher)
        # Whether M is the language we are looking for
        counter_example = teacher.is_language(M, 'one_counter')
        is_language = type(counter_example) is not str and counter_example

        if not is_language:
            add_counterexample_to_RST(counter_example, RST, teacher, alphabet)

    print("Learning took " + str(len(teacher.prev.keys())) + " word queries.")
    return M


if __name__ == "__main__":

    def is_anbn(w):
        counter = 0
        i = 0
        while i < len(w) and w[i] == 'a':
            i += 1
            counter += 1
        while i < len(w) and w[i] == 'b':
            i += 1
            counter -= 1
        return i == len(w) and counter == 0
    
    alphabet = set(['a', 'b'])
    wmapper = {'a': 1, 'b': -1}
    teacher = Teacher(func=is_anbn)

    learn_one_counter([alphabet, wmapper], teacher)


