import itertools

class Branch(object):
    INDEXER = itertools.count(1)
    def __init__(self, name, terms):
        self.name = name
        self.uid = self.INDEXER.next()
        self.terms = terms
    def __repr__(self):
        return "%s(%s) -> %s" % (self.name, self.uid, " ".join(str(t) for t in self.terms))

class Rule(object):
    def __init__(self, name, *branches):
        self.name = name
        self.branches = [Branch(self.name, terms) for terms in branches]
    def __str__(self):
        return self.name
    def __repr__(self):
        branches = " | ".join(" ".join(str(t) for t in b.terms) for b in self.branches)
        return "%s -> %s" % (self.name, branches)
    def add(self, *branches):
        self.branches.extend(Branch(self.name, terms) for terms in branches)
    def __iter__(self):
        return iter(self.branches)

#===============================================================================
# Grammar Rules
#===============================================================================
EPSILON = Rule("<E>")
N = Rule("N", ["fruit"], ["fruits"], ["fly"], ["flies"], ["banana"], ["bananas"])
DET = Rule("DET", ["a"], ["an"], ["the"])
NP = Rule("NP", ["john"], ["bill"], [N], [DET, N])
P = Rule("P", ["on"], ["in"], ["at"], ["with"], ["to"], ["like"])
PP = Rule("PP", [P, NP])
NP.add([NP, PP], [N, NP])
V = Rule("V", ["talk"], ["fly"], ["like"], ["talks"], ["flies"], ["likes"])
VP = Rule("VP", [V], [V, NP], [V, PP])
S = Rule("S", [NP, VP])

#===============================================================================
# Earley
#===============================================================================
class DottedRule(object):
    def __init__(self, branch, dot_index, token_index):
        self.branch = branch
        self.dot_index = dot_index
        self.token_index = token_index
        self.backlinks = []
    def __repr__(self):
        terms = [str(t) for t in self.branch.terms]
        terms.insert(self.dot_index, "$")
        return "%s->%s[%d]" % (self.branch.name, ".".join(terms), self.token_index)
    def __eq__(self, other):
        if not isinstance(other, DottedRule):
            return False
        return self.branch.uid == other.branch.uid and self.dot_index == other.dot_index
    def __ne__(self, other):
        return not (self == other)
    def next_term(self):
        if self.dot_index >= len(self.branch.terms):
            return None
        return self.branch.terms[self.dot_index]
    def match(self, tok):
        term = self.next_term()
        if isinstance(term, str):
            return term == tok
        return False
    def completed(self):
        return self.dot_index >= len(self.branch.terms)

class EarleyParser(object):
    def __init__(self, start, text):
        self.tokens = [None] + text.split()
        self.table = [[] for i in range(len(self.tokens))]
        self.table[0].append(DottedRule(Branch("q0", [start]), 0, 0))
        self.index = 0
    
    def _predict(self):
        column = self.table[self.index]
        for dr in column:
            rule = dr.next_term()
            if isinstance(rule, Rule):
                for branch in rule:
                    dr2 = DottedRule(branch, 0, self.index)
                    if dr2 not in column:
                        column.append(dr2)
    
    def _scan(self):
        tok = self.tokens[self.index+1]
        for dr in self.table[self.index]:
            if dr.match(tok):
                print "!!", dr, "!!", tok
                dr2 = DottedRule(dr.branch, dr.dot_index + 1, dr.token_index)
                self.table[self.index + 1].append(dr2)
    
    def _complete(self):
        for dr in self.table[self.index]:
            if not dr.completed():
                continue
            for dr2 in self.table[dr.token_index]:
                term = dr2.next_term()
                if not isinstance(term, Rule):
                    continue
                if term.name == dr.branch.name:
                    dr3 = DottedRule(dr2.branch, dr2.dot_index + 1, dr2.token_index)
                    dr3.backlinks.append(dr)
                    dr3.backlinks.extend(dr2.backlinks)
                    self.table[self.index].append(dr3)
    
    def parse(self):
        for i in range(len(self.tokens)-1):
            self._predict()
            print self.table[self.index]
            self._scan()
            print self.table[self.index]
            self._complete()
            print self.table[self.index]
            self.index += 1
            print
        
        for dr in self.table[self.index]:
            if dr.branch.name == "q0" and dr.completed():
                return dr
        return None

N = Rule("N", ["fruit"], ["fruits"], ["fly"], ["flies"])
NP = Rule("NP", [N], [DET, N])
NP.add([N, NP])

X = Rule("X", ["x"])
X.add([X, "+", X], [X, "*", X])

ep = EarleyParser(X, "x + x * x")
dr = ep.parse()

if not dr:
    print "failed!"

def print_bp(dr, i=0):
    if dr is None:
        return
    print "  " * i + str(dr)
    for bp in dr.backlinks:
        print_bp(bp, i+1) 

print_bp(dr)

























