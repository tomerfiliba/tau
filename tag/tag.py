class Foot(object):
    def __init__(self, nonterminal):
        self.nonterminal = nonterminal
    def __str__(self):
        return "%s*" % (self.nonterminal,)

EPSILON = ""

class NonTerminal(object):
    def __init__(self, name):
        self.name = name
    def __sub__(self, children):
        return Tree(S, children)
    def __invert__(self):
        return Foot(self)
    def __str__(self):
        return self.name

class Tree(object):
    def __init__(self, root, children):
        self.root = root
        self.children = children
       
    def get(self, path):
        node = self
        for p in path:
            node = node.children[p]
        return node
    
    def show(self, indent = 0):
        print "  " * indent + str(self.root)
        for child in self.children:
            if isinstance(child, Tree):
                child.show(indent + 1)
            else:
                print "  " * (indent + 1) + str(child)

    def is_aux(self):
        return self.find_foot() is not None
    
    def find_foot(self):
        for i, child in enumerate(self.children):
            if isinstance(child, Foot):
                return (i,)
            elif isinstance(child, Tree):
                p = child.find_foot()
                if p is not None:
                    return (i,) + p
        return None

class TAG(object):
    def __init__(self, elementary_trees, start_symbol):
        self.elementary_trees = elementary_trees
        self.start_symbol = start_symbol
        self.by_symbol = {}
        for tree in self.elementary_trees:
            if tree.root not in self.by_symbol:
                self.by_symbol[tree.root] = []
            self.by_symbol[tree.root].append(tree)
        assert self.start_symbol in self.by_symbol
    
    @property
    def init_trees(self):
        return (t for t in self.elementary_trees if not t.is_aux())

    @property
    def aux_trees(self):
        return (t for t in self.elementary_trees if t.is_aux())


LA = "LA"
LB = "LB"
RA = "RA"
RB = "RB"
class State(object):
    def __init__(self, tree, dot_addr, dot_pos, i, j, k, l, sat):
        self.tree = tree
        self.dot_addr = dot_addr
        self.dot_pos = dot_pos
        self.i = i
        self.j = j
        self.k = k
        self.l = l
        self.sat = sat
    def __repr__(self):
        return "(%r, %r, %r, %r, %r, %r, %r, %r)" % (self.tree, self.dot_addr, self.dot_pos, 
            self.i, self.j, self.k, self.l, self.sat)
    def __hash__(self):
        return hash((self.tree, self.dot_addr, self.dot_pos, self.i, self.j, self.k, self.l, 
            self.sat))
    def __eq__(self, other):
        return ((self.tree, self.dot_addr, self.dot_pos, self.i, self.j, self.k, self.l, self.sat) == 
            (other.tree, other.dot_addr, other.dot_pos, other.i, other.j, other.k, other.l, other.sat))
    def __ne__(self, other):
        return not (self == other)
    def at_dot(self):
        node = self.tree.get(self.dot_addr)
        if isinstance(node, Tree):
            return node.root
        else:
            return node
    def is_terminal(self):
        return isinstance(self.at_dot(), str)
    def is_foot(self):
        return isinstance(self.at_dot(), Foot)
    def is_nonterminal(self):
        return isinstance(self.at_dot(), NonTerminal)
    
    def clone(self, dot_addr = None, dot_pos = None, i = None, j = None, k = None, l = None, sat = None):
        return State(self.tree, 
            dot_addr if dot_addr is not None else self.dot_addr,
            dot_pos if dot_pos is not None else self.dot_pos,
            i if i is not None else self.i,
            j if j is not None else self.j,
            k if k is not None else self.k,
            l if l is not None else self.l,
            sat if sat is not None else self.sat)


def parse(grammar, tokens):
    tokens = [None] + list(tokens)
    chart = set()
    for tree in grammar.init_trees:
        if tree.root == grammar.start_symbol:
            chart.add(State(tree, (), LA, 0, None, None, 0, False))
    
    while True:
        before_len = len(chart)
        for st in tuple(chart):
            # scan (1) and (2)
            if st.dot_pos == LA and not st.sat and st.is_terminal():
                if st.at_dot() == tokens[st.l+1]:
                    chart.add(st.clone(dot_pos = RA, l = st.l+1))
                if st.at_dot() == EPSILON:
                    chart.add(st.clone(dot_pos = RA))
            
            # predict (3)
            if st.dot_pos == LA and not st.sat and st.is_nonterminal():
                for t in grammar.by_symbol[st.at_dot()]:
                    chart.add(State(t, (), LA, st.l, None, None, st.l, False))
                # predict (4): if no OA
                if False:
                    chart.add(st.clone(dot_pos = LB))
            
            # predict (5)
            if st.dot_pos == LB and not st.sat and st.is_foot() and st.i == st.l and st.j == st.k == None:
                #for t in grammar.elementary_trees:
                #    pass
                pass
                
        
        if len(chart) == before_len:
            # no more changes -- we're done
            break
    
    print chart





if __name__ == "__main__":
    S = NonTerminal("S")
    t1 = S-["e"]
    t2 = S-["a", S-["c", ~S, "d"], "b"]
    g = TAG([t1, t2], S)
    
    #parse(g, "e")
    #parse(g, "aabbeccdd")











