INIT_TREE = 0
LEFT_AUX = 1
RIGHT_AUX = 2
MODIFIED_TREE = 3

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
    def modify(self, i, child):
        children2 = list(self.children)
        children2[i] = child
        return Tree(self.root, children2, MODIFIED_TREE)
    
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
    def add(self, state, reason, subtrees = None):
        if subtrees is None:
            subtrees = [state.tree]
        self._changes.append((state, reason, subtrees))
    def commit(self):
        added = False
        while self._changes:
            st, reason, subtrees = self._changes.pop(0)
            if st not in self._states:
                st.index = len(self._ordered_states)
                self._ordered_states.append(st)
                coll = self._states[st] = {}
                added = True
            else:
                coll = self._states[st]
            for stree in subtrees:
                if stree in coll:
                    coll[stree].add(reason)
                else:
                    coll[stree] = {reason}

        return added
    
    def get_subtrees(self, st):
        return self._states.get(st, {})
    
    def show(self, only_completed = False):
        for st in self._ordered_states:
            if only_completed and not st.is_complete():
                continue
            subtrees = " ; ".join("%s:%s" % (",".join(reason), t) for t, reason in self.get_subtrees(st).items())
            print "%3d | %-30s | %s" % (st.index, st, subtrees)

#===================================================================================================
# 
#===================================================================================================
def handle_left_aux(grammar, chart, st):
    if st.dot != 0:
        return
    
    # (2)
    for t in grammar.get_left_aux_trees_for(st.tree.root):
        chart.add(State(t, 0, st.j, st.j), "[2/%d]" % (st.index,))
    
    # (3)
    for st2 in chart:
        if st2.tree.type == LEFT_AUX and st2.is_complete() and st2.tree.root == st.tree.root and st.j == st2.i:
            chart.add(State(st.tree, 0, st.i, st2.j), "[3/%d-%d]" % (st.index, st2.index),
                [st2.tree.modify(-1, st.tree)])
                #[t.modify(-1, st.tree) for t in chart.get_subtrees(st2)])

def handle_scan(grammar, chart, st, token):
    prod = st.next()
    if isinstance(prod, str):
        # (4)
        if prod == token:
            chart.add(State(st.tree, st.dot+1, st.i, st.j+1), "[4/%d]" % (st.index,),
                [t for t in chart.get_subtrees(st).keys()])
        # (5)
        elif prod == "":
            chart.add(State(st.tree, st.dot+1, st.i, st.j), "[5/%d]" % (st.index,))
    elif isinstance(prod, Foot):
        # (6)
        chart.add(State(st.tree, st.dot+1, st.i, st.j), "[6/%d]" % (st.index,))

def handle_substitution(grammar, chart, st):
    prod = st.next()
    if isinstance(prod, NonTerminal):
        # (7)
        for t in grammar.get_init_trees_for(prod):
            chart.add(State(t, 0, st.j, st.j), "[7/%d]" % (st.index,))
        
        # (8)
        for st2 in chart:
            if st2.is_complete() and st2.tree.type == INIT_TREE and st.j == st2.i:
                chart.add(State(st.tree, st.dot + 1, st.i, st2.j), "[8/%d-%d]" % (st.index, st2.index),
                    [st.tree.modify(st.dot, t) for t in chart.get_subtrees(st2)])
                    #[t.modify(st.dot, st2.tree) for t in chart.get_subtrees(st)])
 
def handle_subtree_traversal(grammar, chart, st):
    prod = st.next()
    if isinstance(prod, Tree):
        # (9)
        chart.add(State(prod, 0, st.j, st.j), "[9/%d]" % (st.index,))
        
        # (10)
        for st2 in chart:
            if st2.tree == prod and st2.is_complete() and st2.i == st.j:
                chart.add(State(st.tree, st.dot + 1, st.i, st2.j), "[A/%d-%d]" % (st.index, st2.index),
                    #[t.modify(st.dot, st2.tree) for t in chart.get_subtrees(st)])
                    [st.tree.modify(st.dot, st2.tree)]
                    )

def handle_right_aux(grammar, chart, st):
    return
#    if not st.is_complete():
#        return
#    
#    # (11)
#    for t in grammar.get_right_aux_trees_for(st.tree.root):
#        chart.add(State(t, 0, st.j, st.j))
#    
#    # (12)
#    for st2 in chart:
#        if st2.tree.type == RIGHT_AUX and st2.is_complete() and st.j == st2.i and \
#                st2.tree.root == st.tree.root:
#            chart.add(State(st.tree, len(st.tree.children), st.i, st2.j), 
#                [t.modify(0, st.tree) for t in chart.get_subtrees(st2)])


def parse(grammar, start_symbol, tokens):
    chart = Chart()
    tokens = list(tokens)
    padded_tokens = [None] + tokens
    
    # (1)
    for t in grammar.get_init_trees_for(start_symbol):
        chart.add(State(t, 0, 0, 0), "[1]")
    
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
        #NP("john"),
        #NP("mary"),
        #N("apple"),
        N("banana"),
        #N("boy"),
        #N("telescope"),
        #NP(D("a"), N),
        #NP(D("an"), N),
        NP(D("the"), N),
        #S(NP, VP(V("likes"), NP)),
        #S(NP, VP(V("saw"), NP)),
    ],
    aux_trees = [
        #VP(Adv("really"), Foot(VP)),
        N(Adj("tasty"), Foot(N)),
        #N(Foot(N), PP(P("with"), NP)),
        #VP(Foot(VP), PP(P("with"), NP)),
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


# 1, 1, 2, 5, 14, 42, 132, 429
#chart, matches = parse(g2, T, "a + a + a + a".split())

chart, matches = parse(g, NP, "the tasty banana".split())
chart.show()
print

for t in chart.get_subtrees(matches[0]):
    print t







