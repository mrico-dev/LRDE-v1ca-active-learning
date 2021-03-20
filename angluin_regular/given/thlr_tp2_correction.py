from graphviz import Digraph
from display_automaton import export_automaton



# Non-deterministic finite automaton
class NFA:

    def __init__(self, all_states, initial_states, final_states,
                 alphabet, edges):
        # States: a set of integers
        self.all_states = set(all_states)
        # The alphabet: a set of strings
        # "" stands for epsilon
        self.alphabet = set(alphabet)
        if "" in self.alphabet:
            self.alphabet.remove("")
        # Initial and final states: two sets of integers
        self.initial_states = set(initial_states).intersection(self.all_states)
        self.final_states = set(final_states).intersection(self.all_states)
        # There must be an initial state; if there isn't, an initial state 0
        # is added
        if not self.initial_states:
            self.initial_states.add(0)
            self.all_states.add(0)
        # Edges: a dictionnary (origin, letter): set of destinations
        self.next_states = {(state, letter): set()
                            for state in self.all_states
                            for letter in self.alphabet}
        for edge in set(edges):
            if (edge[0] in self.all_states) and (edge[2] in self.all_states) \
                    and (edge[1] in self.alphabet):
                self.next_states[(edge[0], edge[1])].add(edge[2])


if __name__ == "__main__":
    # Testing regular expressions
    a = RegEx("a", [])
    b = RegEx("b", [])
    c = RegEx("c", [])
    a_star = RegEx("*", [a])
    bc = RegEx(".", [b, c])
    e = RegEx("+", [a_star, bc])
    print("Expects (a)*+bc:", e.to_string())

    # Testing Thompson's algorithm
    E = e.to_enfa()
    export_automaton(E, "E")

    # Testing the ENFA class
    A = ENFA([0, 1, 2], [0], [1], ["a", "b"], [(0, "a", 1), (0, "", 1),
                                               (1, "b", 2), (2, "", 0)])
    export_automaton(A, "A")
    A.new_state()
    export_automaton(A, "A_new")

    # Testing the epsilon closure
    print("Expects {0, 1}:", A.epsilon_reachable(0))
    print("Expects {0, 1, 2}:", A.epsilon_reachable(2))

    # Testing the removal of epsilon transitions
    B = A.to_nfa()
    export_automaton(B, "B")
