from graphviz import Digraph
from .display_automaton import export_automaton


def get_dests_from_state(states, edges, src):
    """
    Get all dest which you can directly go from src
    """
    res = []
    for e in edges:
        if e[0] == src:
            res.append(e[2])
    return res


class OneCounter:

    def __init__(self, all_states, initial_states, final_states,
                 alphabet, wmapper, edges):
        self.states = set(all_states)
        self.init_states = set(initial_states)
        self.final_states = set(final_states)
        self.alphabet = set(alphabet)
        self.edges = edges
        self.word_mapper = wmapper


    def remove_state(self, q):
        for e in self.edges:
            if e[0] == q or e[2] == q:
                self.edges.remove(e)
        self.states.remove(q)
        if q in self.init_states:
            self.init_states.remove(q)
        if q in self.final_states:
            self.final_states.remove(q)


    def get_all_dest(self, q):

        dest = get_dests_from_state(self.states, self.edges, q)
        length = 0
        while length != len(dest):

            length = len(dest) 
            for q in dest:
                for q2 in get_dests_from_state(self.states, self.edges, q):
                    if q2 not in dest:
                        dest.append(q2)

        return dest


    def remove_useless_states(self) -> None:

        # Removing one that can never go accepted
        for st in list(self.states):

            if st in self.final_states:
                continue
            
            dest = self.get_all_dest(st)

            if set(dest).intersection(set(self.final_states)) == set():
                self.remove_state(st)
        
        # Removing ones that are not accessible
        all_accessible = set(self.init_states)
        for init in self.init_states:
            dest = self.get_all_dest(init)
            for e in dest:
                all_accessible.add(e)

        for q in list(self.states):
            if q not in all_accessible:
                self.remove_state(q)
        
