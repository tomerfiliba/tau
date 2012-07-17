
from adt.tree import Tree



class AST(Tree):
    pass



class ControlAST(AST):
    
    def __unicode__(self):
        if not self.subtrees:
            return unicode(self.root)
        else:
            return u"%s\n%s" % (self.root, indent(bulleted_list(self.subtrees)))




class ASTDict(dict):
    
    def __unicode__(self):
        return bulleted_list(self._fmt_element(k, v)
                             for k, v in self.iteritems())

    def _fmt_element(self, k, v):
        k = unicode(k) + " -> "
        indent_by = " " * max(10, len(k))
        v = indent(unicode(v), indent_by)
        return k + v[len(k):]

    def __repr__(self):
        return unicode(self)


def bulleted_list(items, bullet="-"):
    return "\n".join(u"%s %s" % (bullet, x) for x in items)

def indent(text, indent="    "):
    return "\n".join(indent + line for line in text.splitlines())
