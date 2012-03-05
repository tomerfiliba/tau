class Node(object):
    def show(self, indentation = 0):
        print "%s%s" % ("  " * indentation, self)

class Tree(Node):
    def __init__(self, root_label, *children):
        self.parent = None
        self.root_label = root_label
        self.children = []
        for child in children:
            self.append(child)
    def append(self, child):
        self.children.append(child)
        child.parent = self
    def __str__(self):
        return self.root_label
    def show(self, indentation = 0):
        Node.show(self, indentation)
        for child in self.children:
            child.show(indentation + 1)

class Terminal(Node):
    def __init__(self, value):
        self.parent = None
        self.value = value
    def __str__(self):
        return repr(self.value)

class AdjuctionPoint(Node):
    def __init__(self):
        self.parent = None
        self.root_label = root_label
    def __str__(self):
        return "%s*" % (self.root_label,)


np = NonTerminal("np", NonTerminal("d"), NonTerminal("n", Terminal("boy")))
d = NonTerminal("d", Terminal("the"))

np.show()
d.show()

