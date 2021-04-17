import pandas as pd
import numpy as np
import re

import given.thlr_tools as thlr
import given.display_automaton as display

class Teacher:

    prev = {}

    def accepts_word(self, word: str) -> bool:
        """
        Tell whether a word is accepted or not
        """
        res = re.match("^[abc]*[defgh]+iii$", word) is not None
        if word not in self.prev.keys():
            self.prev[word] = res
        return res
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

    def is_language(self, automata: thlr.NFA) -> str or True:
        """
        Tell wheter the language is accepted, if not gives a counter example
        """
        display.export_automaton(automata, "learning_automata")

        res = input("Please compare the two automata and give a counter example\
                (or OK if the automata is good): ")

        return True if res == "OK" else res


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


def set_first_table_ownership(SET, alphabet, teacher) -> None:
    """
    Add all alphabet symbol as words into SET
    """
    SET[""][""] = teacher.accepts_word("")

    for c in alphabet:
        if teacher.accepts_word(c):
            SET.at[c, ""] = True
        else:
            SET.at[c, ""] = False


def fill_SET_row(SET, teacher, row_name):
    """
    Fills a of SET row using a teacher
    """
    for col in SET.columns:
        SET.at[row_name, col] = teacher.accepts_word(row_name + col)


def make_SET_consistent(SET, alphabet, teacher) -> None:
    """
    Try to make the SET consistent
    """
    indexes = list(SET.index)
    
    # Adding an element of alphabet to each element of S
    # Looking for s1
    for s in indexes:
        # Checking for every a
        for c in alphabet:
            SET_cp = SET.copy()
            new_s = s + c
            # Processing row(s1.a)
            if new_s not in list(SET.index):
                SET_cp = SET_cp.append(pd.Series(name=new_s, dtype=bool))
                fill_SET_row(SET_cp, teacher, new_s)
            
            # Checking if every other equivalent rows match that
            # Getting s2
            for other_s in indexes:
                if other_s == s:
                    continue
                
                if SET_cp.loc[other_s].eq(SET_cp.loc[s]).values.all():

                    # Getting s2 . a
                    new_other_s = other_s + c
                    if new_other_s not in list(SET_cp.index):
                        SET_cp = SET_cp.append(pd.Series(name=new_other_s, dtype=bool))
                        fill_SET_row(SET_cp, teacher, new_other_s)

                    # Checking if row(s1 . a . e) != row(s2 . a . e)
                    if not SET_cp.loc[new_other_s].eq(SET_cp.loc[new_s]).values.all():
                        res = False
                        for col_name in list(SET.columns):
                            if SET_cp.at[new_other_s, col_name] != SET_cp.at[new_s, col_name]: 
                                # Adding (a . e) to E
                                new_suff = c + col_name
                                if new_suff not in list(SET.columns):
                                    SET[new_suff] = pd.Series(dtype=bool)
                                    # Filling new row (extending T)
                                    for s2 in SET.index:
                                        SET.at[s2, new_suff] = teacher.accepts_word(s2 + new_suff)
                                    return False, SET
            
    return True, SET


def make_SET_closed(SET, alphabet, teacher):
    """
    Try to make the SET closed
    """
    # Getting s1
    for s1 in list(SET.index):
        # Getting a
        for a in alphabet:
            SET_cp = SET.copy()
            # Getting s1 . a
            s1a = s1 + a
            if s1a in list(SET.index):
                continue

            SET_cp = SET_cp.append(pd.Series(name=s1a, dtype=bool))
            fill_SET_row(SET_cp, teacher, s1a)
            
            new_row = True
            # Getting every other S
            for s2 in list(SET.index):
                if s1 == s2:
                    continue
                # checking if s1.a = s2 
                if SET_cp.loc[s1a].eq(SET_cp.loc[s2]).values.all():
                    new_row = False
                    break

            if new_row:
                SET = SET.append(pd.Series(name=s1a, dtype=bool))
                fill_SET_row(SET, teacher, s1a)
                return False, SET

    return True, SET


def add_counterexample_to_SET(ce, SET, teacher):
    """
    Add counter example to S (the rows) and fill them with teacher answer
    """
    for pref in get_all_prefixes(ce):
        if pref not in list(SET.index):
            SET = SET.append(pd.Series(name=pref, dtype=bool))
            fill_SET_row(SET, teacher, pref)

    return SET


def get_S_from_word(SET, word, teacher) -> str:
    """
    Get a state index of SET from a word w of same equivalence class
    """
    if word in list(SET.index):
        return word

    SET = SET.append(pd.Series(name=word, dtype=bool))
    fill_SET_row(SET, teacher, word)
    duplicate = SET[SET.duplicated(keep='last')]

    res = duplicate.index.values[0]
    SET.drop(index=word)

    return res


def SET_to_NFA(SET, alphabet, teacher) -> thlr.NFA:
    """
    Create a NFA from the SET
    """
    print(SET)
    SET = SET.drop_duplicates(inplace=False)

    # Set of state (we use a dict to match str-indexes with state int value)
    indices = {}
    states = []
    i = 0
    for index in list(SET.index):
        indices[index] = i
        states.append(i)
        i += 1

    # Set of init state
    init = [indices[""]]

    # Getting finals
    final = []
    for index in list(SET.index):
        if SET[""][index] == 1:
            final.append(indices[index])

    # Getting transitions
    transitions = []
    for index in list(SET.index):
        for c in alphabet:
            dest_word = index + c
            dest_word_index = get_S_from_word(SET, dest_word, teacher)
            trans = (indices[index], c, indices[dest_word_index])
            if trans not in transitions:
                transitions.append(trans)

    return thlr.NFA(states, init, final, alphabet, transitions)


def process_angluin(alphabet: set, teacher: Teacher) -> thlr.NFA:
    """
    Returns a NFA corresponding to the one described by the teacher
    """
    # SET Table (using panda)
    SET = pd.DataFrame(index=[""], columns=[""], dtype=bool)
    set_first_table_ownership(SET, alphabet, teacher)

    is_language = False
    while not is_language:

        is_consistent = False
        is_closed = False

        while not is_consistent or not is_closed:
            print(SET)
            is_consistent, SET = make_SET_consistent(SET, alphabet, teacher)
            is_closed, SET = make_SET_closed(SET, alphabet, teacher)
            print("is_closed: " + str(is_closed) + ", is_consistent:" + str(is_consistent))

        M = SET_to_NFA(SET, alphabet, teacher)
        # Whether M is the language we are looking for
        counter_example = teacher.is_language(M)
        is_language = type(counter_example) is not str and counter_example == True

        if not is_language:
            SET = add_counterexample_to_SET(counter_example, SET, teacher)

    print("Learning took " + str(len(teacher.prev.keys())) + " word queries.")
    return M

        
if __name__ == "__main__":

    #A = thlr.NFA([0, 1, 2, 3], [0], [3], ["a", "b", "c"], [(0, "a", 1), (0, "a", 2), \
    #        (1, "b", 1), (1, "a", 3), (2, "b", 2), (2, "c", 3)])

    A = thlr.NFA([0, 1], [0], [1], ["a"], [(0, "a", 1)])

    display.export_automaton(A, "expected_result")
    
    automata = process_angluin(set(['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i']), Teacher())

    display.export_automaton(automata, "final")

