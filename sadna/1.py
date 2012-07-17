"""

"""

class Label(object):
    pass
class Radio(object):
    pass
class Var(object):
    pass
class Image(object):
    pass
class Cond(object):
    pass
Otherwise = None
def run(prog):
    pass


v = Var("a")

L = (
    Label("do you like?") . size(100,20)
    ---
    Radio(checked = v) | Label("Yes") | Radio(checked = ~v) | Label("No")
)

I = (
    Image(filename = Cond(v, {True: "like.png", Otherwise: "dislike.png"})) . size(32, 32)
)

main = L | I

run(main)

