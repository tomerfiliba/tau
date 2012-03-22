import operator


class Matrix(object):
    def __init__(self, *rows):
        self.rows = [[float(v) for v in row] for row in rows]
        self.m = len(self.rows)
        self.n = len(self.rows[0])
        self._eliminated = False
        if any(len(row) != self.n for row in self.rows):
            raise ValueError("Not all rows are of equal length")
    def __str__(self):
        return "\n".join(("(%s)" % ("  ".join("%3.2f" % (a,) for a in row))) for row in self.rows)
    def __getitem__(self, (r,c)):
        return self.rows[r][c]
    def __setitem__(self, (r,c), value):
        self.rows[r][c] = value
    def swap_rows(self, i, j):
        self.rows[i], self.rows[j] = self.rows[j], self.rows[i]
    def eliminate(self):
        if self._eliminated:
            return
        for k in range(self.m):
            i_max, val = max(((i, abs(self[i, k])) for i in range(k, self.m)), key = lambda (i,v): v)
            if val == 0:
                continue
            self.swap_rows(k, i_max)
            for i in range(k+1, self.m):
                for j in range(k+1, self.n):
                    self[i,j] = self[i, j] - self[k, j] * (self[i, k] / self[k, k])
                self[i, k] = 0
            for j in range(self.n):
                first = self[k,j]
                if first != 0:
                    for h in range(j, self.n):
                        self[k,h] /= first
                    break
        self.rows.sort()
        self._eliminated = True
    
    def solve(self, variables):
        if len(variables) != self.n - 1:
            raise ValueError("Expected %d variables" % (self.n - 1,))
        self.eliminate()
        assignments = {}
        for row in self.rows:
            nonzero = [v for v in row if v != 0]
            if not nonzero:
                continue
            if len(nonzero) == 1:
                raise ValueError("No solution exists")
            vars = [variables[i] for i in range(self.n - len(nonzero), self.n - 1)]
            assignee = vars.pop(0)
            assert nonzero.pop(0) == 1
            const = nonzero.pop(-1)
            print "!!", assignee, vars, const
            assignments[assignee] = const
            
            for i, v in enumerate(vars):
                if v not in assignments:
                    assignments[v] = FreeVar(v)
                assignments[assignee] -= nonzero[i] * assignments[v]
        return assignments 


class ExprMixin(object):
    def __mul__(self, other):
        return BinExpr(operator.mul, "*", self, other)
    def __sub__(self, other):
        return BinExpr(operator.sub, "-", self, other)
    def __rmul__(self, other):
        return BinExpr(operator.mul, "*", other, self)
    def __rsub__(self, other):
        return BinExpr(operator.sub, "-", other, self)

class BinExpr(ExprMixin):
    def __init__(self, op, opstr, lhs, rhs):
        self.op = op
        self.opstr = opstr
        self.lhs = lhs
        self.rhs = rhs
    def eval(self, freevars):
        lv = self.lhs.eval(freevars) if hasattr(self.lhs, "eval") else self.lhs
        rv = self.rhs.eval(freevars) if hasattr(self.rhs, "eval") else self.rhs
        return self.op(lv, rv)
    def __repr__(self):
        return "(%r %s %r)" % (self.lhs, self.opstr, self.rhs)

class FreeVar(ExprMixin):
    def __init__(self, name):
        self.name = name
    def eval(self, freevars):
        return freevars[self.name]
    def __repr__(self):
        return self.name


if __name__ == "__main__":
    #m = Matrix([1,2,4,2], [3,7,6,6])
    m = Matrix([0,2,4,2])
    m.eliminate()
    print m
    print 
    
    sol = m.solve(["x", "y", "z"])
    print sol
    
    print
    print sol["y"]
    print sol["y"].eval({"z" : 10})
    print

    print sol["x"]
    print sol["x"].eval({"z" : 4})










