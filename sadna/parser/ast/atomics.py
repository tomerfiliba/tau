
from parser.ast import AST



class AtomicWidget(AST):
    
    def __init__(self, kind, attributes, (width, height)):
        super(AtomicWidget, self).__init__(kind, [])
        self.kind = kind
        self.attributes = attributes
        self.width = width
        self.height = height
        
    def __unicode__(self):
        return "(%s:%sx%s)[%s]" % (self.kind, self.width, self.height,
                                   ",".join("%s=%s" % kv for kv in 
                                            self.attributes.iteritems()))
        
    __repr__ = __unicode__
    
    
    
class Literal(AST):
    
    def __init__(self, value):
        super(Literal, self).__init__(value, [])
        
    @property
    def value(self):
        return self.root
    
