from decimal import Decimal
from matrix_solver import Matrix
scalar_types = (int, long, float, Decimal)

class LinearMixin(object):
    def __or__(self, other):
        return LinEq(self, other)
    def __add__(self, other):
        return LinSum(self, other)
    def __radd__(self, other):
        return LinSum(other, self)
    def __sub__(self, other):
        return self + (-1 * other)
    def __neg__(self):
        return -1 * self

    def __mul__(self, scalar):
        return Coeff(self, scalar)
    def __rmul__(self, scalar):
        return Coeff(self, scalar)

class LinEq(object):
    def __init__(self, lhs, rhs):
        if not isinstance(lhs, LinSum):
            lhs = LinSum(lhs)
        if not isinstance(rhs, LinSum):
            rhs = LinSum(rhs)
        self.lhs = lhs
        self.rhs = rhs
    def __repr__(self):
        return "%r = %r" % (self.lhs, self.rhs)

class LinVar(LinearMixin):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name

class Coeff(LinearMixin):
    def __init__(self, var, scalar):
        if not isinstance(scalar, scalar_types):
            raise TypeError("%r is not a scalar" % (scalar,))
        if isinstance(var, Coeff):
            self.var = var.var
            self.coeff = var.coeff * scalar
        else:
            self.var = var
            self.coeff = scalar
    def __repr__(self):
        return "%r%r" % (self.coeff, self.var)

class LinSum(LinearMixin):
    def __init__(self, *elements):
        self.elements = []
        for elem in elements:
            if isinstance(elem, LinSum):
                self.elements.extend(elem.elements)
            elif isinstance(elem, Coeff):
                self.elements.append(elem)
            elif isinstance(elem, LinVar):
                self.elements.append(Coeff(elem, 1))
            elif isinstance(elem, scalar_types):
                self.elements.append(elem)
            else:
                raise TypeError("cannot sum %r" % (elem,))
    def __repr__(self):
        return " + ".join(repr(e) for e in self.elements)
    def __iter__(self):
        return iter(self.elements)

class LinSys(object):
    def __init__(self, *equations):
        self.equations = equations
    def to_matrix(self):
        vars_indexes = {}
        equations = []
        for eq in self.equations:
            vars = [e for e in eq.lhs if isinstance(e, Coeff)] + [-e for e in eq.rhs if isinstance(e, Coeff)]
            scalars = sum([-e for e in eq.lhs if isinstance(e, scalar_types)] + 
                [e for e in eq.rhs if isinstance(e, scalar_types)])
            varbins = {}
            for v in vars:
                if v.var not in vars_indexes:
                    vars_indexes[v.var] = len(vars_indexes)
                if v.var not in varbins:
                    varbins[v.var] = v.coeff
                else:
                    varbins[v.var] += v.coeff
            equations.append((varbins.items(), scalars))
        
        matrix = []
        for i, (vars, scalar) in enumerate(equations):
            row = [0] * (len(vars_indexes) + 1)
            row[-1] = scalar
            for v, coeff in vars:
                row[vars_indexes[v]] = coeff
            matrix.append(row)
        
        return Matrix(*matrix), sorted(vars_indexes.keys(), key = lambda v: vars_indexes[v])
    
    def solve(self):
        matrix, vars = self.to_matrix()
        return matrix.solve([v.name for v in vars])



if __name__ == "__main__":
    a = LinVar("a")
    b = LinVar("b")
    
    ls = LinSys(
        2 * a + 7 * b - 2 | 9 + 2 * b,
        4 * a             | b + 11,
    )
    
    print ls.solve()
    
    


