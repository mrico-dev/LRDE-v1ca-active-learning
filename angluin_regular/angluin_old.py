import given/thrl_tools as thlr
import given/display_automaton as display
import panda as pd


class Teacher:

    def __init__(self, ref_language: thlr.RegEx or thlr.NFA):
        """
        Set the teacher that will automatically answer for a given language
        """
        self.automata = ref_language
        if type(ref_language) is thlr.RegEx:
            self.automata = ref_language.to_enfa()
        else:
            raise TypeError("Wrong type for ref_language")

        self.automata = self.automata.minimization()
        display.export_automaton(self.automata, "ref_automata")


    def accepts_word(self, word: str) -> bool:
        """
        Tell whether a word is accepted or not
        """
        res = ""
        while res != "Y" and res != "N":
            res = input("Is the word '" + word "' accepted? [y/n] ")
            res = res.upper()

        return res == "Y"


    def is_language(self, automata: thrl.NFA) -> str or True:
        """
        Tell wheter the language is accepted, if not gives a counter example
        """
        display.export_automaton(automata.minimization(), "learning_autotmata")

        res = input("Please compare the two automata and give a counter example\
                (or OK if the automata is good): ")

        return (res == "OK") ? True : res


def get_all_suffixes(word: str):
    """
    Get all suffixes of a word
    """
    res = [""]
    suff = ""
    i = len(word) - 1
    while i >= 0:
        suff = word[i] + suff
        res.append(suff)

    return res


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


def add_all_prefixes_to_set(word: str, s: set) -> None:
    """
    Add all prefixes of work to s
    """
    for pref in get_all_prefixes(word):
        s.add(pref)


def set_first_table_ownership(S, E, T, alphabet, teacher) -> None:
    """
    Add all alphabet symbol as words into SET
    """
    T[""][""] = teacher.accepts_word("")

    for c in alphabet:
        if teacher.accepts_word(c):
            S.add(c)
            T.at[c, ""] = True
            T.at["", c] = True
        else:
            T.at[c, ""] = False
            T.at["", c] = False


def is_SET_consitent(S, E, T) -> bool:
    """
    Tell whether SET is consitent
    """
    # TODO

    return False


def make_SET_consitent(S, E, T) -> None:
    """
    Try to make the SET consistent
    """

    # TODO


def is_SET_closed(S, E, T) -> bool:
    """
    Tell whether SET is closed
    """
    # Prefix closed
    for pref_word in S:
        for pref in get_all_prefixes(pref_word):
            if pref not in S:
                return False

    #for suff_word in E:
    #    for suff in get_all_suffixes(suff_word):
    #        if suff not in E:
    #            return False

    return True


def make_SET_closed(S, E, T) -> None:
    """
    Try to make the SET closed
    """
    for pref_word in S:
        for pref in get_all_prefixes(pref_word):
            S.add(pref)

    #for suff_word in E:
    #    for suff in get_all_suffixes(suff_word):
    #        E.add(suff)


def SET_to_NFA(S, E, T) -> thlr.NFA:
    """
    Create a NFA from the SET
    """
    # TODO

    return None


def process_angluin(alphabet: set, teacher: Teacher) -> thlr.NFA:
    """
    Returns a NFA corresponding to the described by the teacher
    """
    # Prefixes
    S = set([""])
    # Suffixes
    E = set([""])
    # NFA
    M = thlr.NFA([], [], [], alphabet, [])
    # Table (using panda)
    T = DataFrame(index=[set([""])], columns=[""])

    set_first_table_ownership(S, E, T, alphabet, teacher)

    # Whether M is the language we are looking for
    is_language = False
    
    while not is_language:

        is_consistent = False
        is_closed = False

        while not is_consistent or not is_closed:

            is_consistent = is_SET_consitent(S, E, T)
            if not is_consistent:
                make_SET_consitent(S, E, T)
            
            is_closed = is_SET_closed(S, E, T)
            if not is_closed:
                make_SET_closed(S, E, T)

        counter_example = teacher.is_language(M)
        is_language = type(counter_example) is not str and counter_example == True

        if not is_language:
            add_all_prefixes_to_set(counter_example, S)
            # TODO extend T to (S U S. A)  E using membership queries. 

    return M

        
