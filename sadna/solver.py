class LayoutCombinator(object):
    pass

class 


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
        















