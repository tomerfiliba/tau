import itertools
from decimal import Decimal

EPSILON = Decimal(1e-20)

class Matrix(object):
    def __init__(self, *rows):
        self.rows = [[Decimal(v) for v in row] for row in rows]
        self.m = len(self.rows)
        self.n = len(self.rows[0])
        if any(len(row) != self.n for row in self.rows):
            raise ValueError("Not all rows are of equal length")
    def __str__(self):
        return "\n".join(("(%s)" % ("  ".join("% 3.2f" % (a,) for a in row))) for row in self.rows)
    def __getitem__(self, ind):
        if isinstance(ind, tuple):
            r, c = ind
            return self.rows[r][c]
        return self.rows[ind]
    def __setitem__(self, ind, value):
        if isinstance(ind, tuple):
            r, c = ind
            self.rows[r][c] = value
        else:
            self.rows[ind] = value
    def eliminate(self):
        """Original code by Jarno Elonen, Public Domain
        http://elonen.iki.fi/code/misc-notes/python-gaussj/index.html"""
        outmatrix = Matrix(*self.rows)
        for y in range(0, self.m):
            maxrow = y
            # Find max pivot
            for y2 in range(y+1, self.m):
                if abs(outmatrix[y2][y]) > abs(outmatrix[maxrow][y]):
                    maxrow = y2
            outmatrix[y], outmatrix[maxrow] = outmatrix[maxrow], outmatrix[y]
            if abs(outmatrix[y][y]) <= EPSILON:
                # Singular
                continue
            # Eliminate column y
            for y2 in range(y+1, self.m):
                c = outmatrix[y2][y] / outmatrix[y][y]
                for x in range(y, self.n):
                    outmatrix[y2][x] -= outmatrix[y][x] * c
    
        # Backsubstitute
        for y in range(self.m-1, -1, -1):
            for i in range(0, self.n):
                if abs(outmatrix[y][i]) > EPSILON:
                    break
            else:
                continue
            c = outmatrix[y][i]
            for y2 in range(0, y):
                for x in range(self.n-1, y-1, -1):
                    outmatrix[y2][x] -= outmatrix[y][x] * outmatrix[y2][i] / c
            # Normalize row y
            for x in range(i, self.n):
                outmatrix[y][x] /= c
        return outmatrix

    def solve(self, variables):
        if len(variables) != self.n - 1:
            raise ValueError("Expected %d variables" % (self.n - 1,))
        matrix = self.eliminate()
        assignments = {}
        
        for row in reversed(matrix.rows):
            nonzero = list(itertools.dropwhile(lambda v: abs(v) <= EPSILON, row))
            if not nonzero:
                continue
            const = nonzero.pop(-1)
            if not nonzero:
                # a row of the form (0 0 ... 0 x) means a contradiction
                raise ValueError("No solutions exist")
            vars = variables[-len(nonzero):]
            assignee = vars.pop(0)
            assert abs(nonzero.pop(0) - 1) <= EPSILON
            assignments[assignee] = const
            
            for i, v in enumerate(vars):
                if v not in assignments:
                    assignments[v] = FreeVar(v)
                assignments[assignee] -= nonzero[i] * assignments[v]
        return assignments

#===================================================================================================
# Linear equations utilities
#===================================================================================================
def sub(a,b):
    return a-b
sub.__name__ = "-"

def mul(a,b):
    return a*b
mul.__name__ = "*"

class ExprMixin(object):
    def __mul__(self, other):
        if isinstance(other, (int, long, float, Decimal)) and abs(other) <= EPSILON:
            return Decimal(0)
        return BinExpr(mul, self, other)
    def __sub__(self, other):
        if isinstance(other, (int, long, float, Decimal)) and abs(other) <= EPSILON:
            return self
        return BinExpr(sub, self, other)
    def __rmul__(self, other):
        if isinstance(other, (int, long, float, Decimal)) and abs(other) <= EPSILON:
            return Decimal(0)
        return BinExpr(mul, other, self)
    def __rsub__(self, other):
        if isinstance(other, (int, long, float, Decimal)) and abs(other) <= EPSILON:
            return self
        return BinExpr(sub, other, self)

class BinExpr(ExprMixin):
    def __init__(self, op, lhs, rhs):
        self.op = op
        self.lhs = lhs
        self.rhs = rhs
    def eval(self, freevars):
        lv = self.lhs.eval(freevars) if hasattr(self.lhs, "eval") else self.lhs
        rv = self.rhs.eval(freevars) if hasattr(self.rhs, "eval") else self.rhs
        return self.op(lv, rv)
    def __repr__(self):
        return "<BinExpr %s>" % (self,)
    def __str__(self):
        return "(%s %s %s)" % (self.lhs, self.op.__name__, self.rhs)

class FreeVar(ExprMixin):
    def __init__(self, name):
        self.name = name
    def eval(self, freevars):
        return freevars[self.name]
    def __repr__(self):
        return "<FreeVar %s>" % (self,)
    def __str__(self):
        return self.name

#===================================================================================================
# Linear equation solver
#===================================================================================================
if __name__ == "__main__":
    m = Matrix([1,2,4,2], [3,7,6,8])
    sol = m.solve(["x", "y", "z"])
    print sol
    print sol["x"].eval({"z" : 10})
    








