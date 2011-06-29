class Rule(object):
    def __init__(self):
        self.replacements = []
    def add(self, *tokens):
        if tokens == (None,):
            self.replacements.append(None)
        else:
            self.replacements.append(tokens)
    
    def bfs(self):
        for tokens in self.replacements:
            if tokens is None:
                yield ""
                continue
            results = [tokens]
            text = []

S = Rule()
Y = Rule()
S.add("a", S, "b")
S.add("b", Y)
S.add(Y, "a")
Y.add("b", Y)
Y.add("a", Y)
Y.add(None)

for i, out in enumerate(S.bfs()):
    print repr(out)
    if i > 10:
        break


