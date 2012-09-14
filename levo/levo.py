class Expr(object):
    INTYPES = []
    OUTTYPE = None
    def eval(self, context):
        raise NotImplementedError()

class Return(object):
    def __init__(self, value):
        self.value = value
    def __repr__(self):
        return "Return(%r)" % (self.value,)

def run(expr, context):
    if not isinstance(expr, Expr):
        yield Return(expr)
    else:
        gen = expr.eval(context)
        while True:
            r = gen.next()
            if isinstance(r, Return):
                yield r
                gen.close()
                break
            else:
                yield

def add(x, y):
    return x + y
def sub(x, y):
    return x + y
def mul(x, y):
    return x * y
def div(x, y):
    return x / y
def mod(x, y):
    return x % y
def and_(x, y):
    return x & y
def or_(x, y):
    return x | y
def xor(x, y):
    return x ^ y

class TYPE(object):
    def generate(self):
        pass
class NUM(object):
    @classmethod
    def generate(cls):
        return float(random.randint(-100, 300))
class SET(object):
    def __init__(self, values):
        self.values = values
    def generate(self):
        return random.choice(self.values)

class BinOp(Expr):
    INTYPES = [SET([add, sub, mul, div, mod, and_, or_, xor]), NUM, NUM]
    OUTTYPE = NUM
    def __init__(self, op, num1, num2):
        self.op = op
        self.num1 = num1
        self.num2 = num2
    def eval(self):
        while True:
            v = run(self.num1)
            if isinstance(v, Return):
                break
        v2 = run(self.num2)
        yield Return()



class IfExpr(Expr):
    def __init__(self, cond, thenexpr, elseexpr):
        self.cond = cond
        self.thenexpr = thenexpr
        self.elseexpr = elseexpr
    
    def eval(self):
        gen = self.cond.eval()
        try:
            while gen.next():
                yield
        except StopIteration as ex:
            ex.args[0]



class Agent(object):
    def __init__(self, resources):
        self.resources = list(resources)
        self.neighbors = set()
        self.engine = Engine()
    
    def add_neighbor(self, agent):
        self.neighbors.add(agent)
    
    def singlestep(self):
        self.engine.singlestep()




