
from parser.ast import AST



class ArithmeticExpression(object):
    
    def __init__(self, (coeff, free)):
        (self.coeff, self.free) = (coeff, free)
    
    def __unicode__(self):
        if not self.coeff:
            return unicode(self.free)
        else:
            return u" ".join("%+.9g%s" % (v, k) for k, v in self.coeff.iteritems()) \
                + (u" %+.9g" % self.free if self.free != 0 else '')
            return unicode((self.coeff, self.free))
    
    __repr__ = __unicode__


class ExpressionAST(AST):
    pass