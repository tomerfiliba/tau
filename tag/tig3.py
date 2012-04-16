INIT_TREE = 0
LEFT_AUX = 1
RIGHT_AUX = 2

class NonTerminal(object):
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return self.name
    def __call__(self, *children):
        return Tree(self, children)

class Foot(object):
    def __init__(self, nonterminal):
        self.nonterminal = nonterminal
    def __str__(self):
        return "%s*" % (self.nonterminal,)

class Tree(object):
    def __init__(self, root, children, type = INIT_TREE):
        self.root = root
        self.children = tuple(children)
        self.type = type
    def __str__(self):
        return "%s(%s)" % (self.root, ", ".join((repr(c) if isinstance(c, str) else str(c)) 
            for c in self.children))
    def __eq__(self, other):
        if not isinstance(other, Tree):
            return False
        return (self.root, self.children) == (other.root, other.children)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return hash((self.root, self.children))
    def modify(self, i, child, type = None):
        children2 = list(self.children)
        children2[i] = child
        return Tree(self.root, children2, self.type if type is None else type)
    
    def leaves(self):
        for c in self.children:
            if isinstance(c, Tree):
                for n in c.leaves():
                    yield n
            else:
                yield c
    
    def show(self, level = 0):
        print "  " * level + str(self.root)
        for c in self.children:
            if isinstance(c, Tree):
                c.show(level + 1)
            else:
                print "  " * (level + 1) + str(c)


class TIG(object):
    def __init__(self, init_trees, aux_trees):
        self.init_trees_by_symbol = {}
        for t in init_trees:
            leaves = list(t.leaves())
            if any(isinstance(n, Foot) for n in leaves):
                raise TypeError("Initial trees cannot contain foot leaves")
            if t.root not in self.init_trees_by_symbol:
                self.init_trees_by_symbol[t.root] = []
            self.init_trees_by_symbol[t.root].append(t)

        self.left_aux_trees_by_symbol = {}
        self.right_aux_trees_by_symbol = {}
        for t in aux_trees:
            leaves = list(t.leaves())
            if len([n for n in leaves if isinstance(n, Foot)]) != 1:
                raise TypeError("Auxiliary trees must contain exactly one foot", t)
            if isinstance(leaves[-1], Foot):
                foot = leaves[-1]
                coll = self.left_aux_trees_by_symbol
                t.type = LEFT_AUX
            elif isinstance(leaves[0], Foot):
                foot = leaves[0]
                coll = self.right_aux_trees_by_symbol
                t.type = RIGHT_AUX
            else:
                raise TypeError("Auxiliary trees must contain either a leftmost or a rightmost foot", t)
            if foot.nonterminal != t.root:
                raise TypeError("The foot of an auxiliary tree must be of the same nonterminal as the root", t)
            
            if t.root not in coll:
                coll[t.root] = []
            coll[t.root].append(t)
    
    def get_init_trees_for(self, symbol):
        return self.init_trees_by_symbol.get(symbol, ())
    def get_left_aux_trees_for(self, symbol):
        return self.left_aux_trees_by_symbol.get(symbol, ())
    def get_right_aux_trees_for(self, symbol):
        return self.right_aux_trees_by_symbol.get(symbol, ())

class State(object):
    def __init__(self, tree, dot, i, j):
        self.tree = tree
        self.dot = dot
        self.i = i
        self.j = j
        self.index = None
    def __str__(self):
        prod = [u"%s\u2193" % (c,) if isinstance(c, NonTerminal) else str(c) 
            for c in self.tree.children]
        prod.insert(self.dot, u"\u00b7")
        return u"%s \u2192 %s,  %r:%r" % (self.tree.root, " ".join(prod), self.i, self.j)
    def __eq__(self, other):
        return (self.tree, self.dot, self.i, self.j) == (other.tree, other.dot, other.i, other.j)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return hash((self.tree, self.dot, self.i, self.j))
    def is_complete(self):
        return self.dot >= len(self.tree.children)
    def next(self):
        if self.is_complete():
            return None
        return self.tree.children[self.dot]

class Chart(object):
    def __init__(self):
        self._states = {}
        self._ordered_states = []
        self._changes = []
    def __iter__(self):
        return iter(self._ordered_states)
    def __len__(self):
        return len(self._ordered_states)
    def __getitem__(self, index):
        return self._ordered_states[index]
    def add(self, state, reason, parent = None):
        self._changes.append((state, reason, parent))
    def commit(self):
        added = False
        while self._changes:
            st, reason, p = self._changes.pop(0)
            if st not in self._states:
                st.index = len(self._ordered_states)
                self._ordered_states.append(st)
                self._states[st] = (st.index, {p : reason})
                added = True
            elif p is None or p.index < self._states[st][0]:
                self._states[st][1][p] = reason
        return added
    
    def show(self, only_completed = False):
        for st in self._ordered_states:
            if only_completed and not st.is_complete():
                continue
            parents = ", ".join("%s:%s" % ("/" if p is None else p.index, r) for p, r in chart._states[st][1].items())
            print "%3d | %-50s | %s" % (st.index, st, parents)

#===================================================================================================
# 
#===================================================================================================
def handle_left_aux(grammar, chart, st):
    if st.dot != 0:
        return
    
    # (2)
    for t in grammar.get_left_aux_trees_for(st.tree.root):
        chart.add(State(t, 0, st.j, st.j), "LA2", st)
    
    # (3)
    for st2 in chart:
        if st2.tree.type == LEFT_AUX and st2.is_complete() and st2.tree.root == st.tree.root and st.j == st2.i:
            chart.add(State(st.tree, 0, st.i, st2.j), "LA3", st2)

def handle_scan(grammar, chart, st, token):
    prod = st.next()
    if isinstance(prod, str):
        # (4)
        if prod == token:
            chart.add(State(st.tree, st.dot+1, st.i, st.j+1), "SC4", st)
        # (5)
        elif prod == "":
            chart.add(State(st.tree, st.dot+1, st.i, st.j), "SC5", st)
    elif isinstance(prod, Foot):
        # (6)
        chart.add(State(st.tree, st.dot+1, st.i, st.j), "SC6", st)

def handle_substitution(grammar, chart, st):
    prod = st.next()
    if isinstance(prod, NonTerminal):
        # (7)
        for t in grammar.get_init_trees_for(prod):
            chart.add(State(t, 0, st.j, st.j), "SU7", st)
        
        # (8)
        for st2 in chart:
            if st2.is_complete() and st2.tree.type == INIT_TREE and st.j == st2.i:
                chart.add(State(st.tree, st.dot + 1, st.i, st2.j), "SU8", st2)

def handle_subtree_traversal(grammar, chart, st):
    prod = st.next()
    if isinstance(prod, Tree):
        # (9)
        chart.add(State(prod, 0, st.j, st.j), "ST9", st)
        
        # (10)
        for st2 in chart:
            if st2.tree == prod and st2.is_complete() and st2.i == st.j:
                 chart.add(State(st.tree, st.dot + 1, st.i, st2.j), "ST10", st2)

def handle_right_aux(grammar, chart, st):
    if not st.is_complete():
        return
    
    # (11)
    for t in grammar.get_right_aux_trees_for(st.tree.root):
        chart.add(State(t, 0, st.j, st.j), "RA11", st)
    
    # (12)
    for st2 in chart:
        if st2.tree.type == RIGHT_AUX and st2.is_complete() and st.j == st2.i and st2.tree.root == st.tree.root:
            chart.add(State(st.tree, len(st.tree.children), st.i, st2.j), "RA12", st2)


def parse(grammar, start_symbol, tokens):
    chart = Chart()
    tokens = list(tokens)
    padded_tokens = [None] + tokens
    
    # (1)
    for t in grammar.get_init_trees_for(start_symbol):
        chart.add(State(t, 0, 0, 0), "IN1")
    
    while True:
        for st in chart:
            handle_left_aux(grammar, chart, st)
            handle_scan(grammar, chart, st, padded_tokens[st.j+1] if st.j+1 < len(padded_tokens) else None)
            handle_substitution(grammar, chart, st)
            handle_subtree_traversal(grammar, chart, st)
            handle_right_aux(grammar, chart, st)
        
        if not chart.commit():
            # no more changes, we're done
            break
    
    matches = [st for st in chart if st.is_complete() and st.i == 0 and st.j == len(tokens)  
        and st.tree.root == start_symbol and st.tree.type == INIT_TREE]
    return chart, matches


#===================================================================================================
# 
#===================================================================================================
S = NonTerminal("S")
NP = NonTerminal("NP")
VP = NonTerminal("VP")
V = NonTerminal("V")
N = NonTerminal("N")
D = NonTerminal("D")
P = NonTerminal("P")
PP = NonTerminal("PP")
Adv = NonTerminal("Adv")
Adj = NonTerminal("Adj")

g = TIG(
    init_trees = [
        NP("john"),
        NP("mary"),
        N("apple"),
        N("banana"),
        N("boy"),
        N("telescope"),
        NP(D("a"), N),
        NP(D("an"), N),
        NP(D("the"), N),
        S(NP, VP(V("likes"), NP)),
        S(NP, VP(V("saw"), NP)),
    ],
    aux_trees = [
        VP(Adv("really"), Foot(VP)),
        N(Adj("tasty"), Foot(N)),
        N(Foot(N), PP(P("with"), NP)),
        VP(Foot(VP), PP(P("with"), NP)),
    ],
)

T = NonTerminal("T")
OP = NonTerminal("OP")
g2 = TIG(init_trees = [
        T("a"), 
        T(T, OP("+"), T)
    ],
    aux_trees = []
)

#chart, matches = parse(g2, T, "a + a + a".split())
#chart, matches = parse(g, NP, "the tasty banana".split())
chart, matches = parse(g, NP, "the tasty tasty banana".split())
#chart.show()
print len(chart), len(matches)

print "============================"

class DerivationTree(object):
    def __init__(self, root, span, children):
        self.root = root
        self.span = span
        self.children = children
    def __str__(self):
        return "%s%s(%s)" % (self.root, self.span, ", ".join(str(c) for c in self.children))
    def show(self, level = 0):
        print "%s%s  %s" % ("    " * level, self.root, self.span)
        for c in self.children:
            if isinstance(c, DerivationTree):
                c.show(level + 1)
            else:
                print "    " * (level + 1) + repr(c)

class Span(object):
    def __init__(self, i, j):
        self.i = i
        self.j = j
    def __repr__(self):
        return "[%s..%s]" % (self.i, self.j)
    def inside(self, enclosing):
        return self.i >= enclosing.i and self.j <= enclosing.j

def extract(path):
    trees = []
    while path:
        s = path.pop(0)
        sp = Span(s.i, s.j)
        children = []
        first = None
        last = None
        for i, t in enumerate(trees):
            if t.span.inside(sp):
                if first is None:
                    first = i
                last = i
                children.append(t)
            elif first is not None:
                break
        
        print "##", s
        
        if children:
            del trees[first:last+1]
            if isinstance(s.tree.children[-1], Foot) and path and path[0].i == s.i:
                print "!!", s
                s2 = path.pop(0)
                sp = Span(s2.i, s2.j)
                children.append(DerivationTree(s.tree.root, Span(s.j, s2.j), s2.tree.children))
            #if isinstance(s.tree.children[0], Foot) and path and path[0].i == s.i:
            #    s2 = path.pop(0)
            #    sp = Span(s2.i, s2.j)
            #    children.append(DerivationTree(s.tree.root, Span(s.j, s2.j), s2.tree.children))
            trees.insert(first, DerivationTree(s.tree.root, sp, children))
        else:
            trees.append(DerivationTree(s.tree.root, sp, s.tree.children))
    
    assert len(trees) == 1
    return trees[0]

#for p in find_paths(chart, matches[0]):
#    print p

def find_paths(chart, st):
    if st.is_complete():
        yield st
    for parent in chart._states[st][1]:
        if parent is not None:
            for st2 in find_paths(chart, parent):
                yield st2
            return

path = list(find_paths(chart, matches[0]))[::-1]
for p in path:
    print p.index, p
print "============================"
tree = extract(path)
tree.show()










