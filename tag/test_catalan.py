from tig5 import NonTerminal, TIG, parse

#===================================================================================================
# Test Grammar #1
# Generate the Catalan sequence, e.g., the number of ways to place brackets in an 
# expression of the form a + a + ... + a
#
# This serves as a "sanity check" that the parser does indeed generate all possible 
# parse trees. This of course is not rigorous enough, since it does not make use of 
# auxiliary trees, but it does mean we cover at least this simple case.
#
# Here we define the simple (CFG-like) grammar 
#     T -> a | T OP T 
#     OP -> +
#
#===================================================================================================
T = NonTerminal("T")
OP = NonTerminal("OP")
g = TIG(init_trees = [
        T("a"), 
        T(T, OP("+"), T),
    ],
    aux_trees = []
)

#===================================================================================================
# Let's print a couple of trees
#===================================================================================================
for i, t in enumerate(parse(g, T, "a + a + a")):
    print "[%d]" % (i + 1,)
    t.show()
    print

#===================================================================================================
# Now let's see we really generate the Catalan sequence (first 10 numbers)
#===================================================================================================
produced = [len(parse(g, T, " + ".join("a" * i).split())) 
    for i in range(1, 11)] 

print "The sequence we got is ", produced
assert produced == [1, 1, 2, 5, 14, 42, 132, 429, 1430, 4862]
print "Hooray!"




