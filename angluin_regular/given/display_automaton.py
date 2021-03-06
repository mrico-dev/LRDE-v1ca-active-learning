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

