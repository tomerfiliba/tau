
#===================================================================================================
# TIG trees
#===================================================================================================
class NonTerminal(object):
    def __init__(self, name):
        self.name = name
    def __call__(self, *children):
        return Node(self, children)
    def __str__(self):
        return str(self.name)

class Foot(object):
    def __init__(self, nonterm):
        self.nonterm = nonterm
    def __str__(self):
        return "%s*" % (self.nonterm,)
    
class Node(object):
    def __init__(self, root, children):
        self.root = root
        self.children = list(children)
    def __str__(self):
        return "%s" % (self.root,)
    
    def traverse_leaves(self):
        for child in self.children:
            if isinstance(child, Node):
                for n in child.traverse_leaves():
                    yield n
            else:
                yield child

LEFT_AUX = 1
RIGHT_AUX = 2


class TIG(object):
    def __init__(self, start_symbol, init_trees, aux_trees):
        self.init_trees = set(init_trees)
        self.aux_trees = set(aux_trees)
        self.start_symbol = start_symbol
        
        for t in self.init_trees:
            if any(isinstance(n, Foot) for n in t.traverse_leaves()):
                raise TypeError("Initial trees must not contain foot leaves", t)

        for t in self.aux_trees:
            leaves = list(t.traverse_leaves())
            if isinstance(leaves[0], Foot):
                t.direction = RIGHT_AUX
                non_feet_leaves = leaves[1:]
                foot = leaves[0]
            elif isinstance(leaves[-1], Foot):
                t.direction = LEFT_AUX
                non_feet_leaves = leaves[:-1]
                foot = leaves[-1]
            else:
                raise TypeError("An Aux tree must contain a foot", t)
            if any(isinstance(n, Foot) for n in non_feet_leaves):
                raise TypeError("The foot of an Aux tree must be either leftmost or rightmost", t)
            if foot.nonterm != t.root:
                raise TypeError("Foot must be labeled the same as the root", t)

        self.by_symbol_init = {}
        for t in self.init_trees:
            if t.root not in self.by_symbol_init:
                self.by_symbol_init[t.root] = []
            self.by_symbol_init[t.root].append(t)

        self.by_symbol_aux = {}
        for t in self.aux_trees:
            if t.root not in self.by_symbol_aux:
                self.by_symbol_aux[t.root] = []
            self.by_symbol_aux[t.root].append(t)
    
    def get_init_trees(self, symbol):
        return self.by_symbol_init.get(symbol, ())

    def get_aux_trees(self, symbol):
        return self.by_symbol_aux.get(symbol, ())


#===================================================================================================
# parser state
#===================================================================================================
class State(object):
    def __init__(self, tree, dot, i, j):
        self.tree = tree
        self.dot = dot
        self.i = i
        self.j = j
    def __hash__(self):
        return hash((self.tree, self.dot, self.i, self.j))
    def __eq__(self, other):
        return (self.tree, self.dot, self.i, self.j) == (other.tree, other.dot, other.i, other.j)
    def __ne__(self, other):
        return not (self == other)
    def __repr__(self):
        prod = ["%s|" % (c,) if isinstance(c, NonTerminal) else str(c) 
            for c in self.tree.children]
        prod.insert(self.dot, "@")
        return "(%s->%s, %r:%r)" % (self.tree.root, " ".join(prod), self.i, self.j)
    def next(self):
        if self.is_complete():
            return None
        return self.tree.children[self.dot]
    def is_complete(self):
        return self.dot >= len(self.tree.children)

class Chart(object):
    def __init__(self):
        self._elements = {}
        self._additions = []
    def __len__(self):
        return len(self._elements)
    def __iter__(self):
        return iter(self._elements)
    def __getitem__(self, key):
        return self._elements[key]
    def add(self, state, prevs):
        self._additions.append((state, prevs))
    def commit(self):
        prev_len = len(self._elements)
        for s, prevs in self._additions:
            if s not in self._elements:
                self._elements[s] = prevs
            else:
                self._elements[s].update(prevs)
        del self._additions[:]
        return prev_len != len(self._elements)


#===================================================================================================
# parser code
#===================================================================================================
def handle_left_aux(grammar, chart, s):
    if s.dot != 0:
        return

    # Left Aux (2)
    for t in grammar.get_aux_trees(s.tree.root):
        if t.direction == LEFT_AUX:
            chart.add(State(t, 0, s.j, s.j), {s})
    
    # Left Aux (3)
    for s2 in chart:
        if (hasattr(s2.tree, "direction") and s.tree.root == s2.tree.root and 
                s2.is_complete() and s2.i == s.j and s2.tree.direction == LEFT_AUX):
            chart.add(State(s.tree, 0, s.i, s2.j), {s, s2})

def handle_scan(grammar, chart, s, tokens):
    if s.is_complete():
        return
    
    if isinstance(s.next(), str): 
        # Scan (4)
        if s.next() == tokens[s.j+1]:
            chart.add(State(s.tree, s.dot+1, s.i, s.j+1), {s})
        
        # Scan (5)
        if s.next() == "":
            chart.add(State(s.tree, s.dot+1, s.i, s.j), {s})

    # Scan (6)
    if isinstance(s.next(), Foot):
        chart.add(State(s.tree, s.dot+1, s.i, s.j), {s})

def handle_substitution(grammar, chart, s):
    if s.is_complete() or not isinstance(s.next(), NonTerminal):
        return
    
    r = s.next()
    
    # Substitution (7)
    for t in grammar.get_init_trees(r):
        chart.add(State(t, 0, s.j, s.j), {s})

    # Substitution (8)
    for s2 in chart:
        if s2.tree.root == r and s2.is_complete() and s2.i == s.j and s2.tree in grammar.init_trees:
            chart.add(State(s.tree, s.dot + 1, s.i, s2.j), {s, s2})

def handle_subtree(grammar, chart, s):
    if s.is_complete() or not isinstance(s.next(), Node):
        return

    r = s.next()
    
    # Subtree (9)
    chart.add(State(r, 0, s.j, s.j), {s})
    
    # Subtree (10)
    for s2 in chart:
        if s2.tree == r and s2.is_complete() and s2.i == s.j:
            chart.add(State(s.tree, s.dot+1, s.i, s2.j), {s, s2})

def handle_right_aux(grammar, chart, s):
    if not s.is_complete():
        return

    # Right Aux (11)
    for t in grammar.get_aux_trees(s.tree.root):
        if t.direction == RIGHT_AUX:
            chart.add(State(t, 0, s.j, s.j), {s})

    # Right Aux (12)
    for s2 in chart:
        if (hasattr(s2.tree, "direction") and s2.tree.root == s.tree.root and 
                s2.is_complete() and s2.i == s.j and s2.tree.direction == RIGHT_AUX):
            chart.add(State(s.tree, s.dot, s.i, s2.j), {s, s2})


def parse(grammar, tokens):
    chart = Chart()
    tokens = [None] + list(tokens)
    for tree in grammar.get_init_trees(grammar.start_symbol):
        chart.add(State(tree, 0, 0, 0), {})
    
    while True:
        for s in chart:
            handle_left_aux(grammar, chart, s)
            handle_scan(grammar, chart, s, tokens)
            handle_substitution(grammar, chart, s)
            handle_subtree(grammar, chart, s)
            handle_right_aux(grammar, chart, s)
        
        if not chart.commit():
            # no more changes, we're done
            break
    
    matches = []
    for s in chart:
        if (s.is_complete() and s.tree.root == grammar.start_symbol and s.i == 0 and 
                s.j == len(tokens) - 1 and s.tree in grammar.init_trees):
            matches.append(s)
    return matches, chart


if __name__ == "__main__":
    S = NonTerminal("S")
    NP = NonTerminal("NP")
    VP = NonTerminal("VP")
    V = NonTerminal("V")
    N = NonTerminal("N")
    D = NonTerminal("D")
    Adv = NonTerminal("Adv")
    Adj = NonTerminal("Adj")
    
    g = TIG(NP, 
        init_trees = [
            NP("john"),
            NP("mary"),
            N("apple"),
            N("banana"),
            NP(D("a"), N),
            NP(D("an"), N),
            NP(D("the"), N),
            S(NP, VP(V("likes"), NP)),
        ],
        aux_trees = [
            VP(Adv("really"), Foot(VP)),
            N(Adj("tasty"), Foot(N)),
        ],
    )
    
    matches, chart = parse(g, "the banana".split())
    
    def f(s, chart, indent = 0):
        print "  " * indent + str(s)
        for s2 in chart[s]:
            f(s2, chart, indent + 1)
    
    for s in matches:
        f(s, chart)
        






