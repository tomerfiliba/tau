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
    def __init__(self, root, children):
        self.root = root
        self.children = tuple(children)
    def __str__(self):
        return "%s(%s)" % (self.root, ", ".join((repr(c) if isinstance(c, str) else str(c)) 
            for c in self.children))
    def __eq__(self, other):
        return (self.root, self.children) == (other.root, other.children)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return hash((self.root, self.children))
    def modify(self, i, child):
        children2 = list(self.children)
        children2[i] = child
        return Tree(self.root, children2)
    
    def leaves(self):
        for c in self.children:
            if isinstance(c, Tree):
                for n in c.leaves():
                    yield n
            else:
                yield c
    
    def show(self, level = 0):
        print "    " * level + str(self.root)
        for c in self.children:
            if isinstance(c, Tree):
                c.show(level + 1)
            else:
                print "    " * (level + 1) + str(c)


class TIG(object):
    def __init__(self, init_trees, aux_trees):
        self.init_trees = set()
        self.init_trees_by_symbol = {}
        self.all_trees_by_symbol = {}
        for t in init_trees:
            leaves = list(t.leaves())
            if any(isinstance(n, Foot) for n in leaves):
                raise TypeError("Initial trees cannot contain foot leaves")
            if t.root not in self.init_trees_by_symbol:
                self.init_trees_by_symbol[t.root] = []
            self.init_trees_by_symbol[t.root].append(t)
            if t.root not in self.all_trees_by_symbol:
                self.all_trees_by_symbol[t.root] = []
            self.all_trees_by_symbol[t.root].append(t)
            self.init_trees.add(t)

        self.aux_trees_by_symbol = {}
        self.right_aux_trees = set()
        self.left_aux_trees = set()
        for t in aux_trees:
            leaves = list(t.leaves())
            if len([n for n in leaves if isinstance(n, Foot)]) != 1:
                raise TypeError("Auxiliary trees must contain exactly one foot", t)
            if isinstance(leaves[0], Foot):
                foot = leaves[0]
                self.left_aux_trees.add(t)
            elif isinstance(leaves[-1], Foot):
                foot = leaves[-1]
                self.right_aux_trees.add(t)
            else:
                raise TypeError("Auxiliary trees must contain either a leftmost or a rightmost foot", t)
            if foot.nonterminal != t.root:
                raise TypeError("The foot of an auxiliary tree must be of the same nonterminal as the root", t)
            if t.root not in self.aux_trees_by_symbol:
                self.aux_trees_by_symbol[t.root] = []
            self.aux_trees_by_symbol[t.root].append(t)
            if t.root not in self.all_trees_by_symbol:
                self.all_trees_by_symbol[t.root] = []
            self.all_trees_by_symbol[t.root].append(t)
    
    def get_init_trees_for(self, symbol):
        return self.init_trees_by_symbol.get(symbol, ())
    def get_aux_trees_for(self, symbol):
        return self.aux_trees_by_symbol.get(symbol, ())
    def get_trees_for(self, symbol):
        return self.all_trees_by_symbol.get(symbol, ())
    def is_init_tree(self, tree):
        return tree in self.init_trees
    def is_aux_tree(self, tree):
        return not self.is_init_tree(tree)

class State(object):
    def __init__(self, tree, dot, i, j):
        self.tree = tree
        self.dot = dot
        self.i = i
        self.j = j
        self.index = -1
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
        self._states = set()
        self._ordered_states = []
        self._changes = []
    def __iter__(self):
        return iter(self._ordered_states)
    def __len__(self):
        return len(self._ordered_states)
    def __getitem__(self, index):
        return self._ordered_states[index]
    def add(self, state):
        self._changes.append(state)
    def commit(self):
        added = False
        while self._changes:
            st = self._changes.pop(0)
            if st not in self._states:
                st.index = len(self._ordered_states)
                self._ordered_states.append(st)
                self._states.add(st)
                added = True
        return added
    
    def show(self, only_completed = False):
        for st in self._ordered_states:
            if only_completed and not st.is_complete():
                continue
            print "%3d)  %-40s" % (st.index, st)


#===================================================================================================
# 
#===================================================================================================
def handle_left_aux(grammar, chart, st):
    pass

def handle_right_aux(grammar, chart, st):
    pass

def handle_scan(grammar, chart, st, token):
    prod = st.next()
    if isinstance(prod, str):
        if prod == token:
            # (4)
            chart.add(State(st.tree, st.dot + 1, st.i, st.j + 1))
        elif prod == "":
            # (5)
            chart.add(State(st.tree, st.dot + 1, st.i, st.j))
    elif isinstance(prod, Foot):
        # (6)
        chart.add(State(st.tree.modify(st.dot, ""), st.dot + 1, st.i, st.j))

def handle_substitution(grammar, chart, st):
    prod = st.next()
    #if isinstance(prod, Tree):
    #    prod = prod.root
    if isinstance(prod, NonTerminal):
        # (7)
        for t in grammar.get_init_trees_for(prod):
            chart.add(State(t, 0, st.j, st.j))
        
        # (8)
        for st2 in chart:
            if st2.tree.root == prod and st2.i == st.j and st2.is_complete(): # and grammar.is_init_tree(st2.tree):
                chart.add(State(st.tree.modify(st.dot, st2.tree), st.dot + 1, st.i, st2.j))

def handle_subtree_traversal(grammar, chart, st):
    prod = st.next()
    if isinstance(prod, Tree):
        # (9)
        chart.add(State(prod, 0, st.j, st.j))
        
        # (10)
        for st2 in chart:
            if prod == st2.tree and st2.is_complete() and st2.i == st.j:
                chart.add(State(st.tree.modify(st.dot, st2.tree), st.dot + 1, st.i, st2.j))


def parse(grammar, start_symbol, tokens):
    chart = Chart()
    tokens = [None] + list(tokens)
    for t in grammar.get_init_trees_for(start_symbol):
        chart.add(State(t, 0, 0, 0))
    
    while True:
        for st in chart:
            handle_left_aux(grammar, chart, st)
            handle_scan(grammar, chart, st, tokens[st.j+1] if st.j+1 < len(tokens) else None)
            handle_substitution(grammar, chart, st)
            handle_subtree_traversal(grammar, chart, st)
            handle_right_aux(grammar, chart, st)
        
        if not chart.commit():
            # no more changes, we're done
            break
    
    matches = [st for st in chart if st.is_complete() and st.i == 0 and st.j == len(tokens) - 1 
        and st.tree.root == start_symbol] # and grammar.is_init_tree(st.tree)]
    return matches, chart





#===================================================================================================
# 
#===================================================================================================

T = NonTerminal("T")
OP = NonTerminal("OP")

g = TIG(init_trees = [
        T("a"), 
        T(T, OP("+"), T)
    ],
    aux_trees = [
    ]
)


matches, _ = parse(g, T, " + ".join(["a"] * 3).split())
for m in matches:
    m.tree.show()












