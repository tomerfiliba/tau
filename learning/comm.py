import random
import operator
from functools import partial


class Node(object):
    ARGCOUNT = None
    def __init__(self, name, func, args):
        self.name = name
        self.func = func
        self.args = args
    @classmethod
    def bind(cls, name, func):
        class subcls(cls):
            def __init__(self, args):
                cls.__init__(self, name, func, args)
        return subcls
    def eval(self, vars):
        return self.func(*(a.eval(vars) for a in self.args))

class UniNode(Node):
    ARGCOUNT = 1
    def __repr__(self):
        return "%s%r" % (self.name, self.args[0])

class BinNode(Node):
    ARGCOUNT = 2
    def __repr__(self):
        return "(%r %s %r)" % (self.args[0], self.name, self.args[1])

class TerNode(Node):
    ARGCOUNT = 3
    def __repr__(self):
        return "(%r ? %r : %r)" % (self.args[0], self.args[1], self.args[2])

class Leaf(object):
    ARGCOUNT = 0

class Variable(Leaf):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
    def eval(self, vars):
        return vars[self.name]

class Const(Leaf):
    def __init__(self, val):
        self.val = val
    def __repr__(self):
        return repr(self.val)
    def eval(self, vars):
        return self.val

leaf_nodes = [Variable("x")] * 5
leaf_nodes.extend(Const(i) for i in range(10))

ternary = TerNode.bind("?:", lambda cond, then, else_: then if cond else else_)
inner_nodes = [
    BinNode.bind("+", operator.add), BinNode.bind("-", operator.sub), BinNode.bind("*", operator.mul), 
    BinNode.bind("/", lambda a, b: a if b == 0 else a / b), 
    BinNode.bind("%", lambda a, b: a if b == 0 else a % b),
    
    BinNode.bind(">>", lambda a, b: a if b <= 0 else a >> b), 
    BinNode.bind("<<", lambda a, b: a if b <= 0 else a << b),
    BinNode.bind("&", operator.and_), BinNode.bind("|", operator.or_), UniNode.bind("~", operator.inv),
     
    BinNode.bind("==", operator.eq), BinNode.bind("!=", operator.ne), BinNode.bind(">", operator.gt), 
    BinNode.bind(">=", operator.ge), BinNode.bind("<", operator.lt), BinNode.bind("<=", operator.le), 
    
    ternary, ternary, ternary, ternary,
]

def generate(depth, leaf_prob = 0.1):
    if depth <= 0 or random.random() <= leaf_prob:
        return random.choice(leaf_nodes)
    else:
        op = random.choice(inner_nodes)
        return op([generate(depth - 1) for _ in range(op.ARGCOUNT)])

expr = generate(5)
print expr
print expr.eval({"x" : 18})


def mix(expr1, expr2):
    _find(expr1)

def extend(expr, depth = 4):
    op = random.choice(inner_nodes)
    args = [expr] + [generate(depth) for _ in range(op.ARGCOUNT - 1)]
    random.shuffle(args)
    return op(args)














