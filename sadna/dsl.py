class H(Box):
    def __init__(self, *boxes):
        Box.__init__(self)
        self.boxes = boxes

class V(Box):
    def __init__(self, *boxes):
        Box.__init__(self)
        self.boxes = boxes

class Spec(Box):
    def __init__(self, box, w, h):
        Box.__init__(self)
        self.box = box
        self.w = w
        self.h = h

class Box(object):
    def __init__(self, atom):
        self.atom = atom
        self.t = None
        self.l = None
        self.b = None
        self.r = None



class Atom(object):
    pass
class Image(Atom):
    def __init__(self, filename):
        self.filename = filename
class Label(Atom):
    def __init__(self, text):
        self.text = text


p = H(
    V(Label("hello"), Label("world")),
    V(Label("foo") | Label("bar"))
)















