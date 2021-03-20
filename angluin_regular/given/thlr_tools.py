from graphviz import Digraph
from .display_automaton import export_automaton

# A tree-based representation of regular expresssions
class RegEx:

    def __init__(self, symbol, children):
        self.symbol = symbol
        self.children = children

    def to_string(self):
        if self.symbol == "*":
            return "(" + self.children[0].to_string() + ")*"
        elif self.symbol == "+":
            return self.children[0].to_string() + "+" \
                + self.children[1].to_string()
        elif self.symbol == ".":
            return self.children[0].to_string() + self.children[1].to_string()
        else:
            return self.symbol

    # Question 4
    # Output an epsilon-NFA equivalent to the regular expression
    def to_enfa(self):
        enfa = ENFA([0, 1], [0], [1], [], [])
        enfa.convert_reg_ex(0, 1, self)
        return enfa


# Non-deterministic finite automata with epsilon transitions
class ENFA:

    def __init__(self, all_states, initial_states, final_states,
                 alphabet, edges):
        # States: a set of integers
        self.all_states = set(all_states)
        # The alphabet: a set of strings
        # "" stands for epsilon
        self.alphabet = set(alphabet)
        self.alphabet.add("")
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
    # Add a new state to the automaton
    def new_state(self):
        new_state = max(self.all_states) + 1
        self.all_states.add(new_state)
        for letter in self.alphabet:
            self.next_states[(new_state, letter)] = set()
        return new_state

    # Question 2
    # Add a new letter 'letter' to the automaton
    def new_letter(self, letter):
        if not letter in self.alphabet:
            for state in self.all_states:
                self.next_states[state, letter] = set()
        self.alphabet.add(letter)

    # Question 3
    # Insert the automaton matched to the regular expression 'reg_ex'
    # between the two states 'origin' and 'destination' according to
    # Thompson's algorithm
    def convert_reg_ex(self, origin, destination, reg_ex):
        if reg_ex.symbol == ".":
            # 2 new states
            mid_state_1 = self.new_state()
            mid_state_2 = self.new_state()
            # 1 new edge
            self.next_states[(mid_state_1, "")].add(mid_state_2)
            # 2 recursive calls
            self.convert_reg_ex(origin, mid_state_1, reg_ex.children[0])
            self.convert_reg_ex(mid_state_2, destination, reg_ex.children[1])
        elif reg_ex.symbol == "+":
            # 4 new states
            high_state_1 = self.new_state()
            high_state_2 = self.new_state()
            low_state_1 = self.new_state()
            low_state_2 = self.new_state()
            #  4 new edges
            self.next_states[(origin, "")].add(high_state_1)
            self.next_states[(high_state_2, "")].add(destination)
            self.next_states[(origin, "")].add(low_state_1)
            self.next_states[(low_state_2, "")].add(destination)
            # 2 recursive calls
            self.convert_reg_ex(high_state_1, high_state_2, reg_ex.children[0])
            self.convert_reg_ex(low_state_1, low_state_2, reg_ex.children[1])
        elif reg_ex.symbol == "*":
            # 2 new states
            mid_state_1 = self.new_state()
            mid_state_2 = self.new_state()
            # 4 new edges
            self.next_states[(origin, "")].add(mid_state_1)
            self.next_states[(mid_state_2, "")].add(destination)
            self.next_states[(origin, "")].add(destination)
            self.next_states[(mid_state_2, "")].add(mid_state_1)
            # 1 recursive call
            self.convert_reg_ex(mid_state_1, mid_state_2, reg_ex.children[0])
        else:
            # 1 new letter
            self.new_letter(reg_ex.symbol)
            # 1 new edge
            self.next_states[(origin, reg_ex.symbol)].add(destination)

    # Question 5
    # Returns the epsilon forward closure of a state 'origin'
    def epsilon_reachable(self, origin):
        reached_states = {origin}
        old_states = set()
        # Using a fixed-point iterative computation
        # Simple to understand but hardly optimal
        while old_states != reached_states:
            old_states = reached_states.copy()
            for state in old_states:
                for destination in self.next_states[(state, "")]:
                    reached_states.add(destination)
        return reached_states

    # Question 6
    # Returns a NFA equivalent to the epsilon NFA by performing a backward
    # removal of epsilon transitions
    def to_nfa(self):
        edges_nfa = []
        new_final = []
        for state in self.all_states:
            for reached in self.epsilon_reachable(state):
                # Computing the new final states
                # May add duplicates: we don't care
                if reached in self.final_states:
                    new_final.append(state)
                for letter in self.alphabet:
                    for target in self.next_states[(reached, letter)]:
                        # Adding the new edges
                        edges_nfa.append((state, letter, target))
        return NFA(self.all_states, self.initial_states, new_final,
                   self.alphabet, edges_nfa)


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

    # Determines if the state 'state' is useful
    def is_useful(self, state):
        is_accessible = any([self.accessible(initial, state) \
            for initial in self.initial_states])
        is_coaccessible = any([self.accessible(state, final) \
            for final in self.final_states])
        return (is_accessible and is_coaccessible)

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

    # Prune the automaton
    def prune(self):
        old_states =self.all_states.copy()
        for state in old_states:
            if not self.is_useful(state):
                self.remove_state(state)

    # Question 1
    # Determines if the NFA is complete
    def is_complete(self):
        return all([(len(self.next_states[(state, letter)]) > 0)
                        for state in self.all_states
                        for letter in self.alphabet])

    # Question 2
    # Determines if the NFA is deterministic
    def is_deterministic(self):
        return (all([(len(self.next_states[(state, letter)]) <= 1)
            for state in self.all_states
            for letter in self.alphabet]) and (len(self.initial_states) < 2))

    # Question 3
    # Returns the set of states reachable from the states in 'origins'
    # by reading the letter 'letter'
    def reachable_set(self, origins, letter):
        target_set = set()
        for state in origins:
            for target in self.next_states[(state, letter)]:
                target_set.add(target)
        return target_set

    # Question 4
    # Returns a DFA equivalent to the NFA
    def determinize(self):
        if self.is_deterministic():
            return self
        power_sets = [self.initial_states.copy()]
        incoming_sets= [self.initial_states.copy()]
        new_edges = []
        # Until there is no new power set to explore
        while incoming_sets:
            # Pick the next power set to explore
            set_of_states = incoming_sets.pop()
            origin_index = power_sets.index(set_of_states)
            # Explore the successors of the power set
            for letter in self.alphabet:
                new_set_of_states = self.reachable_set(set_of_states, letter)
                if not new_set_of_states in power_sets:
                    incoming_sets.append(new_set_of_states.copy())
                    power_sets.append(new_set_of_states.copy())
                target_index = power_sets.index(new_set_of_states)
                new_edges.append((origin_index, letter, target_index))
        # Determine the final states
        final_sets = []
        for set_index in range(0, len(power_sets)):
            if power_sets[set_index].intersection(self.final_states):
                final_sets.append(set_index)
        # Return the DFA
        return NFA(range(0, len(power_sets)), [0], \
            final_sets, self.alphabet, new_edges)

    # Question 5
    # Returns the mirror of the current NFA
    def mirror(self):
        reversed_edges = []
        for state in self.all_states:
            for letter in self.alphabet:
                for next_state in self.next_states[(state, letter)]:
                    reversed_edges.append((next_state, letter, state))  
        return NFA(self.all_states, self.final_states, self.initial_states,
            self.alphabet, reversed_edges)

    # Question 6
    # Returns the minimal DFA matched to the current NFA
    def minimization(self):
        if not self.is_deterministic():
            det = self.determinize()
        else:
            det = self
        det.prune()
        mir_det = det.mirror()
        det_mir_det = mir_det.determinize()
        det_mir_det.prune()
        mir_det_mir_det = det_mir_det.mirror()
        det_mir_det_mir_det = mir_det_mir_det.determinize()
        det_mir_det_mir_det.prune()
        return det_mir_det_mir_det
        

if __name__ == "__main__":
    A = NFA([0, 1, 2, 3], [0], [3], ["a", "b", "c"], [(0, "a", 1), (0, "a", 2), \
        (1, "b", 1), (1, "a", 3), (2, "b", 2), (2, "c", 3)])
    export_automaton(A, "A")

    # Testing determinism and completeness
    print("Expects False:", A.is_deterministic())
    print("Expects False:", A.is_complete())

    # Testing reachability
    print("Expects {1, 2, 3}:", A.reachable_set([0, 1], "a"))

    # Testing determinization
    B = A.determinize()
    export_automaton(B, "B")
    print("Expects True:", B.is_deterministic())
    print("Expects True:", B.is_complete())

    # Testing the mirror
    C = A.mirror()
    export_automaton(C, "C")

    # Testing the minimization
    D = B.minimization()
    export_automaton(D, "D")

