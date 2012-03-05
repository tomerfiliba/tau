class NonTerminal(object):
    def __init__(self, name):
        self.name = name

class Foot(object):
    def __init__(self, nonterm):
        self.nonterm = nonterm

class Tree(object):
    def __init__(self, root, *children):
        self.root = root
        self.children = list(children)
    def __getitem__(self, index):
        return self.children[index]
    def get(self, path):
        node = self
        for index in path:
            node = node[i]
        return node

class AuxTree(Tree):
    pass
class InitTree(Tree):
    pass

class Lex(object):
    def __init__(self, nonterm, term):
        self.nonterm = nonterm
        self.term = term

class DisplayVisitor(object):
    def __init__(self):
        self.level = 0
    def show(self, obj):
        for t in type(obj).mro():
            func = getattr(self, "show_%s" % (t.__name__,), None)
            if func is not None:
                break
        return func(obj)
    def _print(self, text):
        print "   " * self.level + text
    def show_NonTerminal(self, obj):
        self._print(obj.name)
    def show_Foot(self, obj):
        self._print("+%s*" % (obj.nonterm.name,))
    def show_Lex(self, obj):
        self._print("%s: %r" % (obj.nonterm.name, obj.term))
    def show_Tree(self, obj):
        self._print("+%s" % (obj.root.name,))
        self.level += 1
        try:
            for child in obj.children:
                self.show(child)
        finally:
            self.level -= 1
DisplayVisitor = DisplayVisitor()



class EarleyChart(object):
    def __init__(self, tokens):
        self.columns = [EarleyColumn(None)]
        self.columns.extend(EarleyColumn(t) for t in tokens)
        for i in range(0, len(self.columns)-1):
            self.columns[i].next_col = self.columns[i+1]
    def __len__(self):
        return len(self.columns)
    def __getitem__(self, index):
        return self.columns[index]
    def __iter__(self):
        return iter(self.columns)

class EarleyColumn(object):
    def __init__(self, token):
        self.token = token
        self.states = []
        self.next_col = None
    def __len__(self):
        return len(self.states)
    def __getitem__(self, index):
        return self.states[index]
    def __iter__(self):
        return iter(self.states[:])
    def add(self, state):
        if state not in self.states:
            self.states.append(state)

TOP_LEFT = 1
BOT_LEFT = 2
BOT_RIGHT = 3
TOP_RIGHT = 4

class TAGState(object):
    def __init__(self, tree, dot, pos, star):
        self.tree = tree
        self.dot = dot
        self.pos = pos
        self.star = star
    
    def completed(self):
        return self.dot == () and self.pos == TOP_RIGHT and self.star == ()


class TAG(object):
    def __init__(self, start_symbol, elementary_trees):
        self.elementary_trees = elementary_trees
        self.start_symbol = start_symbol
        self.by_symbol = {}
        for t in self.elementary_trees:
            if t.root not in self.by_symbol:
                self.by_symbol[t.root] = []
            self.by_symbol[t.root].append(t)
        assert self.start_symbol in self.by_symbol
    
    def __getitem__(self, symbol):
        self.by_symbol[symbol]
    
    def parse(self, text):
        return self._parse(text.split())
    
    def _parse(self, tokens):
        chart = EarleyChart(tokens)
        for t in self.init_trees:
            chart[0].add(TAGState(t, (), TOP_LEFT, ()))
        for col in chart:
            while True:
                before_len = len(col)
                for state in col:
                    self._advance(chart, col, state)
                if len(col) == before_len:
                    # no more changes, we're done here
                    break
        for state in chart[-1]:
            if s.completed() and isinstance(s.tree, InitTree):
                return True
        return False
    
    def _advance(self, chart, col, state):
        #if state.pos == TOP_LEFT and state.tree.get(state.dot)
        pass










#===================================================================================================
# test
#===================================================================================================
S = NonTerminal("S")
NP = NonTerminal("NP")
VP = NonTerminal("VP")
V = NonTerminal("V")
D = NonTerminal("D")
N = NonTerminal("N")

a1 = InitTree(S,
    NP, 
    Node(VP,
        Lex(V, "loved"), 
        NP
    )
)

a2 = InitTree(NP, "john")
a3 = InitTree(NP, "sue")

b1 = AuxTree(VP,
    Node(V, "has"),
    Foot(VP)
)


if __name__ == "__main__":  
    g = TAG(S, [a1, a2, a3, b1])
    g.parse("john loved mary")





