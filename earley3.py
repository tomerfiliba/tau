class Production(object):
    def __init__(self, *terms):
        self.terms = terms
    def __len__(self):
        return len(self.terms)
    def __getitem__(self, index):
        return self.terms[index]
    def __iter__(self):
        return iter(self.terms)
    def __repr__(self):
        return " ".join(str(t) for t in self.terms)
    def __eq__(self, other):
        if not isinstance(other, Production):
            return False
        return self.terms == other.terms
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return hash(self.terms)

class Rule(object):
    def __init__(self, name, *productions):
        self.name = name
        self.productions = list(productions)
    def __str__(self):
        return self.name
    def __repr__(self):
        return "%s -> %s" % (self.name, " | ".join(repr(p) for p in self.productions))
    def add(self, *productions):
        self.productions.extend(productions)

class State(object):
    def __init__(self, name, production, dot_index, start_column):
        self.name = name
        self.production = production
        self.start_column = start_column
        self.end_column = None
        self.dot_index = dot_index
    def __repr__(self):
        terms = [str(p) for p in self.production]
        terms.insert(self.dot_index, u"$")
        return "%-5s -> %-16s [%s-%s]" % (self.name, " ".join(terms), self.start_column, self.end_column)
    def __eq__(self, other):
        return (self.name, self.production, self.dot_index, self.start_column) == \
            (other.name, other.production, other.dot_index, other.start_column)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return hash((self.name, self.production))
    def completed(self):
        return self.dot_index >= len(self.production)
    def next_term(self):
        if self.completed():
            return None
        return self.production[self.dot_index]

class Column(object):
    def __init__(self, index, token):
        self.index = index
        self.token = token
        self.states = []
        self._unique = set()
    def __str__(self):
        return str(self.index)
    def __len__(self):
        return len(self.states)
    def __iter__(self):
        return iter(self.states)
    def __getitem__(self, index):
        return self.states[index]
    def enumfrom(self, index):
        for i in range(index, len(self.states)):
            yield i, self.states[i]
    def add(self, state):
        if state not in self._unique:
            self._unique.add(state)
            state.end_column = self
            self.states.append(state)
            return True
        return False
    def print_(self, completedOnly = False):
        print "[%s] %r" % (self.index, self.token)
        print "=" * 35
        for s in self.states:
            if completedOnly and not s.completed():
                continue
            print repr(s)
        print

class Node(object):
    def __init__(self, value):
        self.value = value
        self.parent = None
        self.children = []
    def add(self, child):
        self.children.append(child)
        child.parent = self
        return child
    
    def width(self):
        if not self.children:
            return 1
        else:
            return sum(child.width() for child in self.children)

    def duplicate(self):
        root = self.root()
        return root._duplicate(self)

    def _duplicate(self, target):
        duptarget = None
        node2 = Node(self.value)
        for child in self.children:
            child2, target2 = child._duplicate(target)
            node2.add(child2)
            if target2 is not None:
                duptarget = target2
        if self is target:
            duptarget = node2
        return node2, duptarget

    def root(self):
        n = self
        while n.parent is not None:
            n = n.parent
        return n
    def print_(self, level = 0):
        print "  " * level + str(self.value)
        for child in self.children:
            child.print_(level + 1)


def predict(col, rule):
    for prod in rule.productions:
        col.add(State(rule.name, prod, 0, col))

def scan(col, state, token):
    if token != col.token:
        return
    col.add(State(state.name, state.production, state.dot_index + 1, state.start_column))

def complete(col, state):
    if not state.completed():
        return
    for st in state.start_column:
        term = st.next_term()
        if not isinstance(term, Rule):
            continue
        if term.name == state.name:
            col.add(State(st.name, st.production, st.dot_index + 1, st.start_column))

GAMMA_RULE = u"GAMMA"

def parse(rule, text):
    table = [Column(i, tok) for i, tok in enumerate([None] + text.lower().split())]
    table[0].add(State(GAMMA_RULE, Production(rule), 0, table[0]))
    
    for i, col in enumerate(table):
        for state in col:
            if state.completed():
                complete(col, state)
            else:
                term = state.next_term()
                if isinstance(term, Rule):
                    predict(col, term)
                elif i + 1 < len(table):
                    scan(table[i+1], state, term)
        
        col.print_(completedOnly = True)
    
    # find q0 in last table column (otherwise fail)
    for st in table[-1]:
        if st.name == GAMMA_RULE and st.completed():
            q0 = st
            break
    else:
        raise ValueError("parsing failed")
    
    return q0

SYM = Rule("SYM", Production("a"))
OP = Rule("OP", Production("+"), Production("*"))
EXPR = Rule("EXPR", Production(SYM))
EXPR.add(Production(EXPR, OP, EXPR))


def build_trees(state):
    rules = list(enumerate(t for t in state.production if isinstance(t, Rule)))[::-1]
    end_column = state.end_column
    node = Node(state)
    
    for i, r in rules:
        start_column = state.start_column if i == 0 else None
        for st in end_column:
            if st is state:
                break
            if not st.completed():
                continue
            if start_column is not None and st.start_column != start_column:
                continue
            if st.name == r.name:
                node.add(build_trees(st))
                end_column = st.start_column
                break
    
    return node


q0 = parse(EXPR, "a + a + a")

print "-------------------------------"
tree = build_trees(q0)
tree.print_()





