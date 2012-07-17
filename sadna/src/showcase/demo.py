
from parser.layout_parser import parse



subprograms_1 = """
L <- (label)[text="Do you like?"]
     ----------------------------
     (radio)[checked=v] | (label)[text="Yes"] | (radio)[checked=!v] | (label)[text="No"]
I <- (image:32x32)[filename={(v)=>("dislike.png"), otherwise ("dislike.png")}]
"""



print parse(subprograms_1)
