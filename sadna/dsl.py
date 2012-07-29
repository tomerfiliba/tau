import operator


class BoxMixin(object):
    __slots__ = ()
    def __or__(self, other):
        #if isinstance(self, Layout) and self.dir == Layout.H:
        #    self.append(other)
        #    return self
        return Layout(Layout.H, self, other)
    def __sub__(self, other):
        #if isinstance(self, Layout) and self.dir == Layout.V:
        #    self.append(other)
        #    return self
        return Layout(Layout.V, self, other)
    def __neg__(self):
        return self
    def X(self, w, h):
        return BoxSpec(self, w, h)       

class Layout(BoxMixin):
    H = 1
    V = 2
    def __init__(self, dir, *subboxes):
        self.dir = dir
        self.subboxes = list(subboxes)
    def __repr__(self):
        if self.dir == self.H:
            return "(" +" | ".join(repr(b) for b in self.subboxes) + ")"
        else:
            return "\n---\n".join(repr(b) for b in self.subboxes) 
    def append(self, subbox):
        self.subboxes.append(subbox)

class BoxSpec(BoxMixin):
    def __init__(self, box, w, h):
        self.box = box
        self.w = w
        self.h = h
    def __repr__(self):
        return "%s.X(%r, %r)" % (self.box, self.w, self.h)

class BoxAtom(BoxMixin):
    def __repr__(self):
        attrs = ", ".join("%s=%r" % (k, v) for k, v in self.__dict__.items())
        return "%s(%s)" % (self.__class__.__name__, attrs)
class Label(BoxAtom):
    def __init__(self, text):
        self.text = text
class Radio(BoxAtom):
    def __init__(self, checked):
        self.checked = checked
class Image(BoxAtom):
    def __init__(self, filename):
        self.filename = filename
class Button(BoxAtom):
    def __init__(self, text):
        self.text = text
class Tab(BoxAtom):
    def __init__(self, title, box):
        self.title = title
        self.box = box
class TabColl(BoxAtom):
    def __init__(self, *tabs):
        self.tabs = list(tabs)

class ExprMixin(object):
    __slots__ = ()
    def __add__(self, other):
        return BinExpr(operator.add, "+", self, other)
    def __sub__(self, other):
        return BinExpr(operator.sub, "-", self, other)
    def __mul__(self, other):
        return BinExpr(operator.mul, "*", self, other)
    def __div__(self, other):
        return BinExpr(operator.div, "/", self, other)
    def __eq__(self, other):
        return BinExpr(operator.eq, "==", self, other)
    def __ne__(self, other):
        return BinExpr(operator.ne, "!=", self, other)
    def __gt__(self, other):
        return BinExpr(operator.gt, ">", self, other)
    def __ge__(self, other):
        return BinExpr(operator.ge, ">=", self, other)
    def __lt__(self, other):
        return BinExpr(operator.lt, "<", self, other)
    def __le__(self, other):
        return BinExpr(operator.le, "<=", self, other)
    def __invert__(self):
        return UniExpr(operator.inv, "~", self)

class UniExpr(ExprMixin):
    __slots__ = ["op", "opname", "value"]
    def __init__(self, op, opname, value):
        self.op = op
        self.opname = opname
        self.value = value
    def __repr__(self):
        return "%s%r" % (self.opname, self.value)

class BinExpr(ExprMixin):
    __slots__ = ["op", "opname", "lhs", "rhs"]
    def __init__(self, op, opname, lhs, rhs):
        self.op = op
        self.opname = opname
        self.lhs = lhs
        self.rhs = rhs
    def __repr__(self):
        return "(%r %s %r)" % (self.lhs, self.opname, self.rhs)
    def __call__(self, env):
        lhs = self.lhs(env) if callable(self.lhs) else self.lhs
        rhs = self.rhs(env) if callable(self.rhs) else self.rhs
        return self.op(lhs, rhs)

class Var(ExprMixin):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
    def __call__(self, env):
        return env[self.name]

class Cond(ExprMixin):
    def __init__(self, value, choices):
        self.value = value
        self.choices = choices
    def __repr__(self):
        return "Cond(%r, %r)" % (self.value, self.choices)
    def __call__(self, env):
        val = self.value(env) if callable(self.value) else self.value
        if val in self.choices:
            choice = self.choices[val]
        elif Otherwise in self.choices:
            choice = self.choices[Otherwise]
        else:
            raise KeyError(val)
        return choice(env) if callable(choice) else choice

class Otherwise(object):
    def __repr__(self):
        return "Otherwise"
Otherwise = Otherwise()

def run(prog):
    pass



if __name__ == "__main__":
    v = Var("v")
    
    L = (
        Label("do you like?") . X(100,20)
        ---
        Radio(checked = v) | Label("Yes") | Radio(checked = ~v) | Label("No")
    )
    
    I = (
        Image(filename = Cond(v, {True: "like.png", Otherwise: "dislike.png"})) . X(32, 32)
    )
    
    main = L | I
    
    print main


