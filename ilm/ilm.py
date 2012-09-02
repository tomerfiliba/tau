import itertools
from functools import partial

class Atom(object):
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return self.name
    def __eq__(self, other):
        return self.name == other.name if isinstance(other, Atom) else False
    def __ne__(self, other):
        return not (self == other)

class Var(object):
    _counter = itertools.count()
    def __init__(self):
        self.id = _counter.next()
    def __str__(self):
        return "<v%s>" % (self.id,)
    def __eq__(self, other):
        return self.id == other.id if isinstance(other, Var) else False
    def __ne__(self, other):
        return not (self == other)

class NPred(object):
    def __init__(self, name, *args):
        self.name = name
        self.args = args
    def __str__(self):
        return "%s(%s)" % (self.name, ", ".join(str(a) for a in self.args))
    def __eq__(self, other):
        return (self.name, self.args) == (other.name, other.args) if isinstance(other, NPred) else False
    def __ne__(self, other):
        return not (self == other)

john = Atom("john")
mary = Atom("mary")
sue = Atom("sue")
LOVE = partial(NPred, "LOVE")
SEE = partial(NPred, "SEE")
KNOW = partial(NPred, "KNOW")

#print KNOW(john, SEE(mary, sue))





exit()

class Rule(object):
    def __init__(self, name, interp = None):
        self.name = name
        self.interp = interp
    def __str__(self):
        if self.interp:
            return "%s/%s" % (self.name, self.interp)
        else:
            return self.name
    def __div__(self, interp):
        return Rule(self.name, interp)


class Grammar(object):
    def __init__(self, start_symbol = "S"):
        self.start_symbol = start_symbol
        self.rules = {}
    def add(self, rule, production):
        if rule.name not in self.rules:
            self.rules[rule.name] = []
        self.rules[rule.name].append((rule.interp, production))
    def __str__(self):
        return "\n".join(u"%s \u2192 %s" % (r, " ".join(str(e) for e in p)) 
            for prods in self.rules.values() for r, p in prods)

S = Rule("S")
g = Grammar()
g.add(S/SEE(john,mary), ["john_sees_mary"])
g.add(S/SEE(john,sue), ["john_sees_sue"])

#print g

def find_differences(interp1, interp2):
    if type(interp1) is not type(interp2):
        return False
    if isinstance(interp1, )


def generalize(g):
    g2 = Grammar(g.start_symbol)
    for rule, rules in g.rules.items():
        print rule
        for interp, prod in rules:
            print interp, prod
    return g2

generalize(g)











