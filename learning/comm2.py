class Stack(object):
    def __init__(self):
        self._items = []
    def pop(self):
        if not self._items:
            return 0
        return self._items.pop(-1)
    def push(self, elem):
        if isinstance(elem, (tuple, list)):
            self._items.extend(elem)
        elif elem is not None:
            self._items.append(elem)

class INST(object):
    def __init__(self, name, func):
        self.name = name
        self.func = func
    def __repr__(self):
        return self.name

class BIN(INST):
    def eval(self, stack):
        b = stack.pop()
        a = stack.pop()
        stack.push(self.func(a, b))

class UNI(INST):
    def __init__(self, name, func):
        self.name = name
        self.func = func
    def eval(self, stack):
        self.stack.push(self.func(stack.pop()))

class Terminated(Exception):
    pass


INSTRUCTION_SET = [
    BIN("ADD", operator.add),
    BIN("SUB", operator.sub),
    BIN("MUL", operator.mul),
    BIN("DIV", lambda a, b: a if b == 0 else a / b),
    BIN("MOD", lambda a, b: a if b == 0 else a % b),
    BIN("SHL", lambda a, b: a if b <= 0 else a << b),
    BIN("SHR", lambda a, b: a if b <= 0 else a >> b),
    BIN("AND", operator.and_),
    BIN("OR", operator.or_),
    BIN("XOR", operator.xor),
    UNI("INV", operator.inv),
    UNI("POP", lambda a: None),
    UNI("DUP", lambda a: (a, a)),
    UNI("NOP", lambda a: a),
]

class CPU(object):
    def __init__(self):
        self.ip = 0
        self.stack = []
        self.instructions = []
    def execute(self):
        inst = self.instructions[self.ip]
        inst(self.stack)
        self.ip = (self.ip + 1) % len(self.instructions)
