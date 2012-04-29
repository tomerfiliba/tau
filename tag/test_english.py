from tig5 import NonTerminal, Foot, TIG, parse


#===================================================================================================
# A very incomplete grammar for English
#===================================================================================================
S = NonTerminal("S")
NP = NonTerminal("NP")
VP = NonTerminal("VP")
V = NonTerminal("V")
N = NonTerminal("N")
D = NonTerminal("D")
P = NonTerminal("P")
PP = NonTerminal("PP")
Adv = NonTerminal("Adv")
Adj = NonTerminal("Adj")
Conj = NonTerminal("Conj")

g = TIG(
    init_trees = [
        # nouns
        N("apple"),
        N("banana"),
        N("boy"),
        N("telescope"),
        N("ideas"),
        N("glasses"),
        
        # NP's
        NP("john"),
        NP("mary"),
        NP("bill"),
        
        NP(D("some"), N),
        NP(D("a"), N),
        NP(D("an"), N),
        NP(D("the"), N),
        
        # some intransitive verbs
        VP(V("ran")),
        VP(V("sleep")),
        
        # some transitive verbs
        VP(V("kissed"), NP),
        VP(V("hugged"), NP),
        VP(V("ate"), NP),
        VP(V("saw"), NP),
        
        # S itself is non lexicalized, but this is required to allow VP-level adjunction
        S(NP, VP),
    ],
    aux_trees = [
        # some adjectives
        N(Adj("nice"), Foot(N)),
        N(Adj("little"), Foot(N)),
        N(Adj("tasty"), Foot(N)),
        N(Adj("colorless"), Foot(N)),
        N(Adj("green"), Foot(N)),
        
        # some adverbs
        VP(Adv("really"), Foot(VP)),
        Adj(Adv("very"), Foot(Adj)),
        VP(Foot(VP), Adv("quickly")),
        VP(Foot(VP), Adv("furiously")),
        
        # PP's can adjoin both N's and VP's
        N(Foot(N), PP(P("with"), NP)),
        VP(Foot(VP), PP(P("with"), NP)),
        
        # conjunction (at V, N, NP and VP levels)
        NP(Foot(NP), Conj("and"), NP),
        N(Foot(N), Conj("and"), N),
        VP(Foot(VP), Conj("and"), VP),
        V(V, Conj("and"), Foot(V)),
    ],
)


#===================================================================================================
# All sorts of sentences we can be derived from this grammar (the last two are ambiguous)
# which we use to test the parser
#===================================================================================================
sentences = [
    # adjunction
    "john saw the boy",
    "john saw the nice boy",
    "john saw the nice little boy",
    "john ate the very tasty apple",
    "some colorless green ideas sleep furiously",
    
    # conjunction
    "john and mary ate the banana",
    "john ate the banana and the apple",
    "john ate the banana and apple",
    "john ate the apple and saw the boy",
    "john kissed and hugged mary",
    
    # ambiguity
    "john saw the nice boy with the telescope",
    "john saw the boy with the glasses and the telescope",
]

for text in sentences:
    print "==============================================================="
    print text
    print "==============================================================="
    trees = parse(g, S, text.split())
    for i, t in enumerate(trees):
        print "(%d)" % (i + 1,)
        t.show()
        print

