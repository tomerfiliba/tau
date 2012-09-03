import itertools


class Atom(object):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name

class Pred(object):
    def __init__(self, name, argnum):
        self.name = name
        self.argnum = argnum
    def __repr__(self):
        return "#%s%s" % (self.name, self.argnum)
    def __call__(self, *args):
        if len(args) != self.argnum:
            raise TypeError("wrong number of arguments", self.argnum, len(args))
        return (self,) + args

class Var(object):
    _counter = itertools.count()
    def __init__(self):
        self.id = self._counter.next()
    def __repr__(self):
        return "$%s" % (self.id,)
    def __eq__(self, other):
        return True #isinstance(other, Var)
    def __ne__(self, other):
        return False #not isinstance(other, Var)

class DiffError(Exception):
    pass

def _predicate_diff(pred1, pred2):
    if type(pred1) is not type(pred2):
        raise DiffError("wrong types")
    if isinstance(pred1, (list, tuple)):
        if len(pred1) != len(pred2):
            raise DiffError("incompatible lengths")
        the_repl = None
        the_newvar = None
        out = []
        for p1, p2 in zip(pred1, pred2):
            new, newvar, repl = _predicate_diff(p1, p2)
            if repl is not None:
                if the_repl is not None:
                    raise DiffError("more than a single difference", the_repl, repl)
                the_repl = repl
                the_newvar = newvar
            out.append(new)
        return tuple(out), the_newvar, the_repl
    elif pred1 == pred2:
        return pred1, None, None
    else:
        v = Var()
        return v, v, (pred1, pred2)

def predicate_diff(pred1, pred2):
    new, newvar, repl = _predicate_diff(pred1, pred2)
    if not repl:
        raise DiffError("no diff")
    return new, newvar, repl


#x = SEE(bill, mary)
#y = LOVE(bill, mary)
#z = SEE(bill, sue)
#print predicate_diff(x, y)
#print predicate_diff(x, z)

class Rule(object):
    _counter = itertools.count()
    def __init__(self, name, pred = None):
        id = self._counter.next()
        if name is None:
            name = "R%s" % (id,)
        self.name = name
        self.pred = pred
    
    def __repr__(self):
        if self.pred:
            return "%s/%s" % (self.name, self.pred)
        return str(self.name)
    def __div__(self, pred):
        return Rule(self.name, pred)
    def __hash__(self):
        return hash((self.name, self.pred))
    def __eq__(self, other):
        return (self.name, self.pred) == (other.name, other.pred) if isinstance(other, Rule) else False  
    def __ne__(self, other):
        return not (self == other)  

class Grammar(object):
    def __init__(self):
        self.rules = {}
    def __str__(self):
        return "\n".join("%s -> %s" % (rulepred, prod) 
            for prodpairs in self.rules.values() for rulepred, prod in prodpairs)
    def add(self, rule, prod):
        if rule.name not in self.rules:
            self.rules[rule.name] = []
        self.rules[rule.name].append((rule, prod))
    def remove(self, rule, prod):
        if rule.name not in self.rules:
            return
        self.rules[rule.name].remove((rule, prod))
    def items(self):
        return self.rules.items()

def shortest_string_diff(subs, prod1, prod2):
    str1 = "".join(p for p in prod1 if isinstance(p, str))+"*"
    str2 = "".join(p for p in prod2 if isinstance(p, str))+"*"
    if str1 == str2:
        raise DiffError("no diff")
    diffs = []
    for i in range(len(str1)):
        for j in range(i,len(str1)):
            for k in range(len(str2)):
                for l in range(k,len(str2)):
                    if str1[:i]+str1[j:] == str2[:k]+str2[l:]:
                        diffs.append((str1[i:j], str2[k:l], (i,j)))
    
    diffs.sort(key = lambda (a,b, _): len(a)+len(b))
    if not diffs:
        raise DiffError("no diff")
    min = len(diffs[0][0]) + len(diffs[0][1])
    shortest = list(itertools.takewhile(lambda (a,b,_): len(a)+len(b) == min, diffs))
    if len(shortest) != 1:
        raise DiffError("too many/too few shortest diffs", shortest)
    
    origstr1 = str1[:-1]
    origstr2 = str2[:-1]
    str1, str2, (i, j) = shortest[0]
    if str1 == origstr1:
        raise DiffError("whole string differs")

    outprod = []
    curr = 0
    for p in prod1:
        if not isinstance(p, str):
            outprod.append(p)
            continue
        if curr <= i and curr + len(p) >= i:
            before = p[:i-curr]
            if before:
                outprod.append(before)
            outprod.append(subs)
            curr += len(p)
            if curr > j:
                outprod.append(p[-(curr-j):])
        elif curr >= i and curr + len(p) < j:
            curr += len(p)
    
    return outprod, str1, str2

#N = Rule("N")
#print shortest_string_diff(N, ["john eats sausages"], ["tiger eats sausages"])
#print shortest_string_diff(N, ["bill see sue"], ["bill love sue"])
#print shortest_string_diff(N, ["bill see john"], ["bill see sue"])

def merge(grammar):
    changed = False
    #for _, productions in grammar.items():
    #    pairs = list(itertools.combinations(productions.items(), 2))
    #    for (rulepred1, prod1), (rulepred2, prod2) in pairs:
    #        if rulepred1 == rulepred2 and prod1 == prod2:
    #            grammar.remove(rulepred2)
    return changed

def chunk(grammar):
    changed = False
    for _, productions in grammar.items():
        pairs = list(itertools.combinations(productions, 2))
        for (rulepred1, prod1), (rulepred2, prod2) in pairs:
            try:
                newpred, newvar, (repl1, repl2) = predicate_diff(rulepred1.pred, rulepred2.pred)
            except DiffError:
                continue
            r = Rule(None)
            try:
                newprod, str1, str2 = shortest_string_diff(r/newvar, prod1, prod2)
            except DiffError:
                changed = chunk_one_sided(grammar, rulepred1, prod1, rulepred2, prod2, 
                    newpred, repl1, repl2)
            else:
                grammar.add(r / repl1, str1)
                grammar.add(r / repl2, str2)
                grammar.remove(rulepred1, prod1)
                grammar.remove(rulepred2, prod2)
                grammar.add(rulepred1/newpred, newprod)
                changed = True
    return changed

def chunk_one_sided(grammar, rulepred1, prod1, rulepred2, prod2, newpred, repl1, repl2):
    return False

def subsume_rules(grammar):
    changed = True
    while changed:
        changed = False
        changed |= chunk(grammar)
        changed |= merge(grammar)

def learn(pairs):
    g = Grammar()
    S = Rule("S")
    for pred, surface in pairs:
        g.add(S/pred, surface)
    subsume_rules(g)
    return g

class DeadEnd(Exception):
    pass

def generate(grammar, symbol, pred):
    relevant_rules = [(rulepred.pred, prod) 
        for rulepred, prod in grammar.rules[symbol.name]]
    for rulepred, prod in relevant_rules:
        if pred == rulepred:
            print "!!", pred, rulepred, prod
            try:
                output = []
                for elem in prod:
                    if isinstance(elem, str):
                        output.append(elem)
                    else:
                        print "@@@", repr(rulepred)
                        index = [i for i, rp in enumerate(rulepred) 
                            if isinstance(rp, Var) and rp.id == elem.pred.id][0] 
                        output.append(generate(grammar, elem, pred[index]))
                return "".join(output)
            except DeadEnd:
                pass
    raise DeadEnd()


#john = Atom("john")
#bill = Atom("bill")
#sue = Atom("sue")
#mary = Atom("mary")
#SEE = Pred("SEE", 2)
#LOVE = Pred("LOVE", 2)
#KNOW = Pred("KNOW", 2)
#SAY = Pred("SAY", 2)
#
#S = Rule("S")
#N = Rule("N")
#V = Rule("V")
#VP = Rule("VP")
#VV = Rule("VV")
#x1 = Var()
#x2 = Var()
#x3 = Var()
#x4 = Var()
#x5 = Var()
#x6 = Var()
#
#g = Grammar()
##g.add(S/SEE(bill, sue), ["billseesue"])
##g.add(S/LOVE(bill, sue), ["billlovesue"])
##subsume_rules(g)
#
#g.add(S/(x2, x1, x3), [N/x1, V/x2, N/x3])
#g.add(S/(x5, x4, x6), [N/x4, VV/x5, S/x6])
#g.add(N/bill, "bill")
#g.add(N/sue, "sue")
#g.add(N/john, "john")
#g.add(V/LOVE, "love")
#g.add(V/SEE, "see")
#g.add(VV/KNOW, "know")
#g.add(VV/SAY, "say")
#
#print g
#print "============================"
#
#print generate(g, S, LOVE(bill, sue))
#print generate(g, S, SAY(bill, KNOW(john, LOVE(bill, sue))))







