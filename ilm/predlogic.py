import itertools

class Atom(object):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
    
class Pred(Atom):
    def __init__(self, name, argnum):
        Atom.__init__(self, name)
        self.argnum = argnum
    def __call__(self, *args):
        assert len(args) == self.argnum
        lst = [self]
        lst.extend(args)
        return lst

class Var(object):
    _counter = itertools.count()
    def __init__(self):
        self.id = self._counter.next()
    def __repr__(self):
        return "$%s" % (self.id,)

class DiffError(Exception):
    pass

def _differentiate(pred1, pred2):
    if type(pred1) is not type(pred2):
        raise DiffError("wrong types")
    if isinstance(pred1, list):
        if len(pred1) != len(pred2):
            raise DiffError("incompatible lengths")
        the_repl = None
        out = []
        for p1, p2 in zip(pred1, pred2):
            new, repl = _differentiate(p1, p2)
            if repl is not None:
                if the_repl is not None:
                    raise DiffError("more than a single difference", the_repl, repl)
                the_repl = repl
            out.append(new)
        return out, the_repl
    elif isinstance(pred1, Var):
        return pred1, None
    elif pred1 == pred2:
        return pred1, None
    else:
        return Var(), (pred1, pred2)

def differentiate(pred1, pred2):
    new, repl = _differentiate(pred1, pred2)
    if not repl:
        raise DiffError("no diff")
    return new, repl

john = Atom("john")
bill = Atom("bill")
sue = Atom("sue")
mary = Atom("mary")
SEE = Pred("SEE", 2)
LOVE = Pred("LOVE", 2)
KNOW = Pred("KNOW", 2)

x = SEE(bill, mary)
y = LOVE(bill, mary)
print differentiate(x, y)

class Rule(object):
    def __init__(self, name, pred = None):
        self.name = name
        self.pred = pred
    def __repr__(self):
        if self.pred:
            return "%s/%s" % (self.name, self.pred)
        return str(self.name)
    def __div__(self, pred):
        return Rule(self.name, pred)

def shortest_different_string(prod1, prod2):
    str1 = "".join(p for p in prod1 if isinstance(p, str))
    str2 = "".join(p for p in prod2 if isinstance(p, str))    
    if str1 == str2:
        raise DiffError("no diff")
    diffs = []
    for i in range(len(str1)):
        for j in range(i,len(str1)):
            for k in range(len(str2)):
                for l in range(k,len(str2)):
                    if str1[:i]+str1[j:] == str2[:k]+str2[l:]:
                        diffs.append((str1[i:j], str2[k:l]))
    if len(diffs) != 1:
        raise DiffError("too many/too few diffs", diffs)
    return diffs[0]

S = Rule("S")
[S/SEE(bill, sue), ["billseesue"]]

print shortest_different_string(["billseesue"], ["billlovesue"])





















