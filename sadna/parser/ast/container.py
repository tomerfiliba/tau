
from parser.ast import ControlAST



class ContainerAST(ControlAST):
    
    @property
    def direction(self):
        return self.root
    
    @property
    def subelements(self):
        return self.subtrees
