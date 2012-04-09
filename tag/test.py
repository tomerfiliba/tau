from tig import NonTerminal, TIG, Foot


S = NonTerminal("S")
NP = NonTerminal("NP")
VP = NonTerminal("VP")
V = NonTerminal("V")
N = NonTerminal("N")
D = NonTerminal("D")
Adv = NonTerminal("Adv")
Adj = NonTerminal("Adj")

g = TIG(
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

def get_paths(s, chart):
    if s is None:
        return
    yield s
    for s2 in get_paths(list(chart[s])[0][0], chart):
        if s2.is_complete():
            yield s2    

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
        
        if children:
            del trees[first:last+1]
            if isinstance(s.tree.children[-1], Foot) and path and path[0].i == s.i:
                s2 = path.pop(0)
                sp = Span(s2.i, s2.j)
                children.append(DerivationTree(s.tree.root, Span(s.j, s2.j), s2.tree.children))
            trees.insert(first, DerivationTree(s.tree.root, sp, children))
        else:
            trees.append(DerivationTree(s.tree.root, sp, s.tree.children))
    
    assert len(trees) == 1
    return trees[0]

#matches, chart = g.parse(NP, "the tasty banana".split())
##chart.show()
#path = list(get_paths(matches[0], chart))[::-1]
#print path
#tree = extract(path)
#tree.show()

T = NonTerminal("T")
OP = NonTerminal("OP")

g2 = TIG(
    init_trees = [
        T("x"),
        T(T, OP, T),
        OP("+"),
    ],
    aux_trees = []
)

matches, chart = g2.parse(T, "x + x + x".split())
chart.show()
path = list(get_paths(matches[0], chart))[::-1]
print path

tree = extract(path)
tree.show()

