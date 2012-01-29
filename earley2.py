class Production(object):
    def __init__(self, *terms):
        self.terms = tuple(t.lower() if isinstance(t, str) else t for t in terms)
    def __str__(self):
        return " ".join(str(t) for t in self.terms)
    def __len__(self):
        return len(self.terms)
    def __getitem__(self, index):
        return self.terms[index]
    def __hash__(self):
        return hash(self.terms)
    def __eq__(self, other):
        if not isinstance(other, type(self)):
            return False
        return self.terms == other.terms
    def __ne__(self, other):
        return not (self == other)

class Rule(object):
    def __init__(self, name, *productions):
        self.name = name
        self.productions = list(productions)
    def __str__(self):
        return self.name
    def __repr__(self):
        return "%s -> %s" % (self.name, " | ".join(str(r) for r in self.productions))
    def add(self, *productions):
        self.productions.extend(productions)
    def __len__(self):
        return len(self.productions)
    def __iter__(self):
        return iter(self.productions)

Epsilon = Production()

class State(object):
    def __init__(self, name, production, dot_index, start_column):
        self.name = name
        self.production = production
        self.dot_index = dot_index
        self.start_column = start_column
        self.end_colum = None
        self.backlinks = []
    def __str__(self):
        terms = list(self.production.terms)
        terms.insert(self.dot_index, "$")
        return "%-5s -> %-20s  [%s, %s]" % (self.name, " ".join(str(t) for t in terms), self.start_column, self.end_colum)
    def __hash__(self):
        return hash(self.production)
    def __eq__(self, other):
        if not isinstance(other, type(self)):
            return False
        return (self.name, self.production, self.dot_index, self.start_column) == \
            (other.name, other.production, other.dot_index, other.start_column)
    def __ne__(self, other):
        return not (self == other)
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
    def __str__(self):
        return "#%d" % (self.index,)
    def print_(self):
        print "[#%s] %r" % (self.index, self.token)
        print "--------------------------------------------"
        for r in self:
            if r.completed():
                print r
        print
    def add(self, state):
        if state not in self.states:
            state.end_colum = self
            self.states.append(state)
            return True
        return False
    def __iter__(self):
        return iter(self.states)

def _predict(col, state):
    rule = state.next_term()
    if not isinstance(rule, Rule):
        return False
    changed = False
    for prod in rule:
        if col.add(State(rule.name, prod, 0, col)):
            changed = True
    return changed

def _scan(col2, state, term):
    if term == col2.token:
        col2.add(State(state.name, state.production, state.dot_index + 1, state.start_column))

def _complete(col, state):
    if not state.completed():
        return False
    changed = False
    for state2 in state.start_column:
        t = state2.next_term()
        if isinstance(t, Rule) and t.name == state.name:
            st3 = State(state2.name, state2.production, state2.dot_index + 1, state2.start_column)
            col.add(st3)
            if col.add(st3):
                changed = True

    return changed

def _handle_epsilon(col):
    changed = True
    while changed:
        changed = False
        for state in col:
            if _complete(col, state):
                changed = True
            _predict(col, state)

def parse(root, text):
    table = [Column(i, tok) for i, tok in enumerate([None] + text.lower().split())]
    table[0].add(State("q0", Production(root), 0, table[0]))
    
    for i in range(0, len(table)):
        col = table[i]
        for state in col:
            if state.completed():
                _complete(col, state)
            else:
                term = state.next_term()
                if isinstance(term, Rule): 
                    _predict(col, state)
                elif isinstance(term, str) and i+1 < len(table): 
                    _scan(table[i+1], state, term)
                
        _handle_epsilon(col)
        table[i].print_()
    
    for state in table[-1]:
        if state.name == "q0" and state.completed() and state.start_column is table[0]:
            q0 = state
            break
    else:
        raise Exception("parsing failed")
    
    #return build_trees(table, q0)


#EXPR = Rule("EXPR", Production("x"))
#EXPR.add(Production(EXPR, "+", EXPR), Production(EXPR, "*", EXPR))

Q = Rule("Q", Production("+"), Production("*"))
F = Rule("F", Production("x"))
E = Rule("E", Production(F))
E.add(Production(E, Q, E))
root = parse(E, "x + x * x")     # (x+x)*x, x+(x*x)

def build_trees(table, state, level = 0):
    print "  " * level + str(state)
    rules = [t for t in state.production[::-1] if isinstance(t, Rule)]
    index = state.end_colum
    for st in table[index]:
        if not st.completed() or st is state:
            continue
        rule = rules.pop(0)
        if st.name != rule.name:
            continue
        build_trees(st, level + 1)

#print "==========================================="
#build_trees(root)





















