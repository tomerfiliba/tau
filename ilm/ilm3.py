import random
import itertools


class Atom(object):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
    def equate(self, other):
        return 

class Pred(object):
    def __init__(self, name, argnum, highorder = False):
        self.name = name
        self.argnum = argnum
        self.highorder = highorder
    def __repr__(self):
        return "#%s%s%s" % (self.name, "*" if self.highorder else "", self.argnum)
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

def _substitute_rule(grammar, new, old):
    for _, productions in grammar.items():
        for rulepred, prod in productions:
            for i, p in enumerate(prod):
                if p == old:
                    prod[i].name = new.name

def merge(grammar):
    changed = False
    all_rules = []
    removed = []
    for _, productions in grammar.items():
        all_rules.extend(productions)
    
    for (rulepred1, prod1), (rulepred2, prod2) in itertools.combinations(all_rules, 2):
        if (rulepred1, prod1) in removed or (rulepred2, prod2) in removed:
            continue
        if rulepred1.pred == rulepred2.pred and prod1 == prod2:
            removed.append((rulepred2, prod2))
            grammar.remove(rulepred2, prod2)
            _substitute_rule(grammar, rulepred1, rulepred2)
            
    return changed

def chunk(grammar):
    changed = False
    removed = []
    for _, productions in grammar.items():
        pairs = list(itertools.combinations(productions, 2))
        for (rulepred1, prod1), (rulepred2, prod2) in pairs:
            if (rulepred1, prod1) in removed or (rulepred2, prod2) in removed:
                continue
            try:
                newpred, newvar, (repl1, repl2) = predicate_diff(rulepred1.pred, rulepred2.pred)
            except DiffError:
                continue
            r = Rule(None)
            try:
                newprod, str1, str2 = shortest_string_diff(r/newvar, prod1, prod2)
            except DiffError:
                changed |= chunk_one_sided(grammar, rulepred1, prod1, rulepred2, prod2, 
                    newpred, repl1, repl2)
            else:
                grammar.add(r / repl1, [str1])
                grammar.add(r / repl2, [str2])
                grammar.remove(rulepred1, prod1)
                grammar.remove(rulepred2, prod2)
                grammar.add(rulepred1/newpred, newprod)
                removed.append((rulepred1, prod1))
                removed.append((rulepred2, prod2))
                changed = True
    return changed

def chunk_one_sided(grammar, rulepred1, prod1, rulepred2, prod2, newpred, repl1, repl2):
    return False

def subsume_rules(grammar):
    changed = True
    #print grammar
    while changed:
        #print "===================================="
        changed = False
        changed |= chunk(grammar)
        changed |= merge(grammar)
        #print grammar


'''
S = Rule("S")
g = Grammar()
g.add(S/LOVE(bill,mary), ["bill love mary"])
g.add(S/LOVE(bill,sue), ["bill love sue"])
g.add(S/LOVE(john,mary), ["john love mary"])
g.add(S/LOVE(john,sue), ["john love sue"])
g.add(S/SEE(bill,mary), ["bill see mary"])
g.add(S/SEE(bill,sue), ["bill see sue"])
g.add(S/SEE(john,mary), ["john see mary"])
g.add(S/SEE(john,sue), ["john see sue"])

subsume_rules(g)

g.add(S/KNOW(john, SEE(bill,mary)), ["john know bill see mary"])
g.add(S/KNOW(john, SEE(bill,sue)), ["john know bill see sue"])
g.add(S/KNOW(bill, SEE(john,mary)), ["bill know john see mary"])
g.add(S/KNOW(bill, SEE(john,sue)), ["bill know john see sue"])

subsume_rules(g)

g.add(S/KNOW(john, SEE(bill,mary)), ["john know bill love mary"])
g.add(S/KNOW(john, SEE(bill,sue)), ["john know bill love sue"])
g.add(S/KNOW(bill, SEE(john,mary)), ["bill know john love mary"])
g.add(S/KNOW(bill, SEE(john,sue)), ["bill know john love sue"])

subsume_rules(g)

g.add(S/THINK(john, SEE(bill,mary)), ["john think bill love mary"])
g.add(S/THINK(john, SEE(bill,sue)), ["john think bill love sue"])
g.add(S/THINK(bill, SEE(john,mary)), ["bill think john love mary"])
g.add(S/THINK(bill, SEE(john,sue)), ["bill think john love sue"])

subsume_rules(g)

g.add(S/THINK(john, SEE(mary,bill)), ["john think mary love bill"])
g.add(S/THINK(john, SEE(mary,sue)), ["john think mary love sue"])
g.add(S/THINK(bill, SEE(sue,mary)), ["bill think sue love mary"])
g.add(S/THINK(bill, SEE(sue,sue)), ["bill think sue love bill"])

subsume_rules(g)
'''

class DeadEnd(Exception):
    pass

def generate(grammar, symbol, pred):
    relevant_rules = [(rulepred.pred, prod) 
        for rulepred, prod in grammar.rules.get(symbol.name, ())]
    for rulepred, prod in relevant_rules:
        if pred == rulepred and isinstance(rulepred, (list, tuple)):
            try:
                output = []
                for elem in prod:
                    if isinstance(elem, str):
                        output.append(elem)
                    else:
                        index = [i for i, rp in enumerate(rulepred) 
                            if isinstance(rp, Var) and rp.id == elem.pred.id]
                        if len(index) != 1:
                            raise DeadEnd()
                        output.append(generate(grammar, elem, pred[index[0]]))
                return "".join(output)
            except DeadEnd:
                pass
    raise DeadEnd()

def random_word():
    return "".join(random.choice("abcdefghijklmnopqrstuvwxyz") for _ in range(random.randint(3,7)))

def build_random_predicate(predicates, atoms, nesting):
    p = random.choice(predicates)
    atoms = list(atoms)
    random.shuffle(atoms)
    args = atoms[:p.argnum]
    if p.highorder and p.argnum >= 2:
        if nesting > 0:
            args[1] = build_random_predicate(predicates, atoms, nesting - 1)
        else:
            return build_random_predicate(predicates, atoms, nesting)
    return p(*args)


def produce(grammar, symbol, predicates, atoms, count, nesting = 1):
    utterances = {}
    while count:
        count -= 1
        pred = build_random_predicate(predicates, atoms, nesting)
        if pred in utterances:
            continue
        try:
            utter = generate(grammar, symbol, pred)
        except DeadEnd:
            utter = random_word()
        utterances[pred] = utter
    
    return utterances


atoms = [
    Atom("john"),
    Atom("bill"),
    Atom("mary"),
    Atom("jill"),
]
predicates = [
    Pred("LOVE", 2),
    Pred("SEE", 2),
    Pred("TALK", 2),
    Pred("INTRODUCE", 3),
]









