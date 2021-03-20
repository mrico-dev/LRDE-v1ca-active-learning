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

    # Question 1
    # Returns the set of states reachable from the state 'origin'
    # by reading the input 'word'
    def reachable_states(self, origin, word):
        if not word:
            return set([origin])
        else:
            # Direct successors
            mid_states = set(self.next_states[(origin, word[0])])
            # Recursive application
            target_states = set()
            for mid in mid_states:
                for target in self.reachable_states(mid, word[1:]):
                    target_states.add(target)
            return target_states

    # Question 2
    # Determines if the automaton accepts the word 'word'
    def accepts(self, word):
        for origin in self.initial_states:
            if self.final_states & self.reachable_states(origin, word):
                return True
        return False

    # Question 3
    # Determines if the state 'target' is reachable from the state 'origin'
    def accessible(self, origin, target):
        visited = set()
        incoming = set()
        incoming.add(origin)
        while (len(incoming) > 0):
            current = incoming.pop()
            visited.add(current)
            if current == target:
                return True
            else:
                for letter in self.alphabet:
                    for next_state in self.next_states[(current, letter)]:
                        if not (next_state in visited):
                            incoming.add(next_state)
        return False

    # Question 4
    # Determines if the state 'state' is useful
    def is_useful(self, state):
        is_accessible = any([self.accessible(initial, state) \
            for initial in self.initial_states])
        is_coaccessible = any([self.accessible(state, final) \
            for final in self.final_states])
        return (is_accessible and is_coaccessible)

    # Question 5
    # Remove the state 'state' from the automaton
    def remove_state(self, state):
        if state in self.all_states:
            self.all_states.remove(state)
            for letter in self.alphabet:
                del self.next_states[(state, letter)]
                for origin in self.all_states:
                    if state in self.next_states[(origin, letter)]:
                        self.next_states[(origin, letter)].remove(state)
            if state in self.initial_states:
                self.initial_states.remove(state)
            if state in self.final_states:
                self.final_states.remove(state)

    # Question 6
    # Prune the automaton
    def prune(self):
        old_states =self.all_states.copy()
        for state in old_states:
            if not self.is_useful(state):
                self.remove_state(state)

if __name__ == "__main__":
    A = NFA([0, 1, 2, 3, 4], [0], [2], ["a", "b"], [(0, "a", 1), (1, "a", 2), \
        (1, "b", 1), (0, "b", 3), (3, "a", 3), (4, "a", 2)])
    export_automaton(A, "A")

    # Testing acceptation
    print("Expects True:", A.accepts("abba"))
    print("Expects False:", A.accepts("bb"))
    print("Expects False:", A.accepts("ba"))

    # Testing reachability
    print("Expects True:", A.accessible(0,2))
    print("Expects False:", A.accessible(3,2))

    # Testing usefulness
    print("Expects True:", A.is_useful(0))
    print("Expects True:", A.is_useful(1))
    print("Expects True:", A.is_useful(2))
    print("Expects False:", A.is_useful(3))
    print("Expects False:", A.is_useful(4))

    # Testing pruning
    A.prune()
    export_automaton(A, "A_pruned")

