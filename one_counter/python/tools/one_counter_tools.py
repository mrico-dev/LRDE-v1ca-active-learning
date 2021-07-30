from graphviz import Digraph
from .display_automaton import export_automaton


def get_edges_from_state(edges, src):
    """
    Get all edges which initial state is src
    """
    res = []
    for e in edges:
        if e[0] == src:
            res.append(e)
    return res


def get_next(states, edges, src, a):
    """
    Get the transtion from state src using symbol a
    """
    for e in get_edges_from_state(edges, src):
        if e[1] == a:
            return e[2]
    return None


def get_prev(states, edges, dest, a):
    """
    Get the reverse transition from state dest to src using a
    """
    for e in edges:
        if e[2] == dest and e[1] == a:
            return e[0]

    return None


def get_dests_from_state(states, edges, src):
    """
    Get all dest which you can directly go from src
    """
    res = []
    for e in edges:
        if e[0] == src:
            res.append(e[2])
    return res


def get_all_states_of_level(graph, cv):
    """
    Returns all states index with counter value cv
    """
    res = []
    for q in graph.states:
        if int(q[:(q.index('_'))]) == cv:
            res.append(q)
    return res


def _is_state_isomorphic(g1, g2, state1, state2):
    """
    Returns wether state1 in g1 is isomorphic to state2 in g2
    """
    queue1 = [state1]
    queue2 = [state2]
    visited = set()

    # Specific case of non isomorphism
    if (state1 in g1.final_states) ^ (state2 in g2.final_states):
        return False

    while queue1 != [] and queue2 != []:

        q1 = queue1.pop()
        q2 = queue2.pop()

        visited.add(q1)

        for a in g1.alphabet:
            next1 = get_next(g1.states, g1.edges, q1, a)
            next2 = get_next(g2.states, g2.edges, q2, a)

            if (next1 is None) ^ (next2 is None):
                return False
            if (next1 in g1.final_states) ^ (next2 in g2.final_states):
                return False


            if next1 not in visited and next1 not in queue1:
                queue1.insert(0, next1)
                queue2.insert(0, next2)

            prev1 = get_prev(g1.states, g1.edges, q1, a)
            prev2 = get_prev(g2.states, g2.edges, q2, a)

            if (prev1 is None) ^ (prev2 is None):
                return False
            if (prev1 in g1.final_states) ^ (prev2 in g2.final_states):
                return False

            if prev1 not in visited and prev1 not in queue1:
                queue1.insert(0, prev1)
                queue2.insert(0, prev2)

    return True


def _is_isomorphic(g1, g2, starting_states_1, starting_states_2, couple):
    """
    Recursive function for is_isomorphic
    Checks if every state of starting_states_1 is isomorphic to one of the state of starting_states_2
    Fill the couple list of isomorphic states
    starting_states_1 and starting_states_2 must be the same size
    """
    if starting_states_1 == [] and starting_states_2 == []:
        return True

    for q1 in starting_states_1:
        for q2 in starting_states_2:

            # Let's check if q1 and q2 is a good couple
            if _is_state_isomorphic(g1, g2, q1, q2):

                # Creating new list of staring states withour the couple we just found
                starting_states_1_cp = list(starting_states_1)
                starting_states_2_cp = list(starting_states_2)
                starting_states_1_cp.remove(q1)
                starting_states_2_cp.remove(q2)

                res = _is_isomorphic(g1, g2, starting_states_1_cp, starting_states_2_cp, couple)
                if res:
                    couple.append((q1, q2))
                return res

    return False


def is_isomorphic(g1, g2, level, k):
    """
    Tell whether g1 is isomorphic to g2
    If yes, returns a list of couples
    If not, returns none
    """
    starting_states_1 = get_all_states_of_level(g1, level)
    starting_states_2 = get_all_states_of_level(g2, level + k)

    if len(starting_states_1) != len(starting_states_2):
        return None

    res = []
    if _is_isomorphic(g1, g2, starting_states_1, starting_states_2, res):
        return res

    return None


class OneCounter:


    def __init__(self, all_states, initial_states, final_states,
                 alphabet, wmapper, edges):
        self.states = set(all_states)
        self.init_states = set(initial_states)
        self.final_states = set(final_states)
        self.alphabet = set(alphabet)
        self.edges = edges
        self.word_mapper = wmapper
        self.periodic = False
        self.period_cv = -1
        self.edge_color = {} # init, loopin_nocond, loopin_cond, loopout 


    def display(self):
        """
        Display the one counter states and transitions
        """
        print("AUTOMATA:")
        print("states: " + str(self.states))
        print("edges: " + str(self.edges))
        print("init: " + str(self.init_states))
        print("final: " + str(self.final_states))


    def remove_state(self, q):
        for e in list(self.edges):
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
        
