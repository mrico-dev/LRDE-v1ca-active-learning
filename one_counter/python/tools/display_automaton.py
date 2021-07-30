from graphviz import Digraph


# Display the NFA or ENFA 'automaton' and exports it to 'file'.pdf
def export_automaton(automaton, file):
    graph = Digraph(filename=file)
    for state in automaton.all_states:
        node_shape = 'circle'
        node_colour = 'white'
        if state in automaton.final_states:
            node_shape = 'doublecircle'
        if state in automaton.initial_states:
            node_colour = 'lightgrey'
        graph.attr('node', shape=node_shape, style='filled', \
                fillcolor=node_colour)
        graph.node(str(state))
    for state in automaton.all_states:
        for letter in automaton.alphabet:
            for next_state in automaton.next_states[(state, letter)]:
                edge_colour = 'black'
                if letter == "":
                    edge_colour = 'grey'
                graph.edge(str(state), str(next_state), label=letter, \
                    color=edge_colour)
    graph.render()


def export_one_counter(oc, filename):
    """
    Export a one counter automata
    """
    filename = "./automata/" + filename
    graph = Digraph(filename=filename)
    # State are the same
    for state in oc.states:
        node_shape = 'circle'
        node_colour = 'white'
        if state in oc.final_states:
            node_shape = 'doublecircle'
        if state in oc.init_states:
            node_colour = 'lightgrey'
        graph.attr('node', shape=node_shape, style='filled', \
                fillcolor=node_colour)
        graph.node(str(state))

    for src, a, dest in oc.edges:
        # Symbol +/-
        symbol = "+" if oc.word_mapper[a] == 1 else "-" if oc.word_mapper[a] == -1 else ""

        # edge coloration
        edge_color = "black"
        loopmode = ""
        if oc.periodic and (src, a, dest) in oc.edge_color["loopin_no_cond"]:
            loopmode = ""
            edge_color = "gold4"
        elif oc.periodic and (src, a, dest) in oc.edge_color["loopin_cond"]:
            loopmode = " if cv > " + str(oc.period_cv)
            edge_color = "red"
        elif oc.periodic and (src, a, dest) in oc.edge_color["loopout"]:
            loopmode = " if cv <= " + str(oc.period_cv)
            edge_color = "blue"

        graph.edge(str(src), str(dest), label=str(a) + symbol + loopmode, color=edge_color)

    graph.render()

