class NonTerminal(object):
    def __init__(self, name):
        self.name = name
    def __call__(self, *children):
        return Tree(self, children)
    def __str__(self):
        return str(self.name)

class Foot(object):
    def __init__(self, nonterm):
        self.nonterm = nonterm
    def __str__(self):
        return "%s*" % (self.nonterm,)
    
class Subs(object):
    def __init__(self, nonterm):
        self.nonterm = nonterm
    def __str__(self):
        return "%s!" % (self.nonterm,)

class Tree(object):
    def __init__(self, root, children):
        self.root = root
        self.children = list(children)
    def traverse(self):
        yield self.root
        for child in self.children:
            if isinstance(child, Tree):
                for n in child.traverse():
                    yield n
            else:
                yield child
    
    def traverse_leaves(self):
        for child in self.children:
            if isinstance(child, Tree):
                for n in child.traverse_leaves():
                    yield n
            else:
                yield child
    
    def is_left_aux(self):
        return isinstance(list(self.traverse_leaves())[-1], Foot)
    def is_right_aux(self):
        return isinstance(list(self.traverse_leaves())[0], Foot)

class State(object):
    def __init__(self, tree, dot, i, j):
        self.tree = tree
        self.production = list(self.tree.traverse())
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
        return "<%r, %r, %r, %r>" % (self.tree, self.dot, self.i, self.j)
    def next(self):
        if self.dot >= len(self.production):
            return None
        return self.production[self.dot]
    def is_complete(self):
        return self.dot >= len(self.production)

class Chart(object):
    def __init__(self):
        self._elements = set()
    def __repr__(self):
        return repr(self._elements)
    def add(self, state):
        self._elements.add(state)
    def __len__(self):
        return len(self._elements)
    def __iter__(self):
        return iter(self._elements)

class TIG(object):
    def __init__(self, init_trees, aux_trees, start_symbol):
        self.init_trees = init_trees
        self.aux_trees = aux_trees
        self.start_symbol = start_symbol
        self.by_symbol = {}
        for trees in [self.init_trees, self.aux_trees]:
            for t in trees:
                if t.root not in self.by_symbol:
                    self.by_symbol[t.root] = []
                self.by_symbol[t.root].append(t)
    def is_init(self, tree):
        return tree in self.init_trees
    def is_aux(self, tree):
        return tree in self.aux_trees


def parse(grammar, tokens):
    chart = Chart()
    tokens = [None] + list(tokens)
    for tree in grammar.by_symbol[grammar.start_symbol]:
        chart.add(State(tree, 0, 0, 0))
    
    prev_length = -1
    while len(chart) != prev_length:
        prev_length = len(chart)
        print "!!", chart
        
        for s in tuple(chart):
            # Left (2)
            print "(2)"
            if s.dot == 0:
                for t in grammar.by_symbol[s.tree.root]:
                    if t.is_left_aux():
                        chart.add(State(t, 0, s.j, s.j))
            
            # Left (3)
            print "(3)"
            if s.dot == 0:
                for s2 in chart:
                    if s.tree.root == s2.tree.root and s2.is_complete() and s2.i == s.j and s2.tree.is_left_aux():
                        chart.add(State(s.tree, 0, s.i, s2.j))
            
            # Scan (4)
            print "(4)"
            if not s.is_complete() and isinstance(s.next(), str) and s.next() == tokens[s.j+1]:
                chart.add(State(s.tree, s.dot+1, s.i, s.j+1))
            
            # Scan (5)
            print "(5)"
            if not s.is_complete() and isinstance(s.next(), str) and s.next() == "":
                chart.add(State(s.tree, s.dot+1, s.i, s.j))
    
            # Scan (6)
            print "(6)"
            if not s.is_complete() and isinstance(s.next(), Foot):
                chart.add(State(s.tree, s.dot+1, s.i, s.j))
    
            # Subs (7)
            print "(7)"
            if not s.is_complete() and isinstance(s.next(), Subs):
                r = s.next().nonterm
                for t in grammar.by_symbol[r]:
                    if grammar.is_init(t):
                        chart.add(State(t, 0, s.j, s.j))
    
            # Subs (8)
            print "(8)"
            if not s.is_complete() and isinstance(s.next(), Subs):
                r = s.next().nonterm
                for s2 in chart:
                    if s2.tree.root == r and s2.is_complete() and s2.i == s.j and grammar.is_init(s2.tree):
                        chart.add(State(s.t, s.dot + 1, s.i, s2.k))
            
            # Subtree (9)
            print "(9)"
            if not s.is_complete() and isinstance(s.next(), NonTerminal):
                for t in grammar.by_symbol[s.next()]:
                    chart.add(State(t, 0, s.j, s.j))
            
            # Subtree (10)
            print "(10)"
            if not s.is_complete() and isinstance(s.next(), NonTerminal):
                r = s.next()
                for s2 in chart:
                    if s2.tree.root == r and s2.is_complete() and s2.i == s.j:
                        chart.add(State(s.tree, s.dot+1, s.i, s2.j))
            
            # Right (11)
            print "(11)"
            if s.is_complete():
                for t in grammar.by_symbol[s.tree.root]:
                    if t.is_right_aux():
                        chart.add(State(t, 0, s.j, s.j))
            
            # Right (12)
            print "(12)"
            if s.is_complete():
                for s2 in chart:
                    if s2.tree.root == s.tree.root and s2.is_complete() and s2.i == s.j and s2.tree.is_right_aux():
                        chart.add(State(s.tree, s.dot, s.i, s2.j))
    
    for s in chart:
        if s.is_complete() and s.tree.root == grammar.start_symbol and s.i == 0 and s.j == n and grammar.is_init(s.tree):
            return True
    
    return False
        



S = NonTerminal("S")
NP = NonTerminal("NP")
VP = NonTerminal("VP")
V = NonTerminal("D")

i1 = NP("john")
i2 = NP("bananas")
i3 = S(NP, VP(V("likes"), NP))

g = TIG([i1, i2, i3], [], S)
print parse(g, "john likes bananas".split())






























