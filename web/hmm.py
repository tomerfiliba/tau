from nltk.tag.hmm import HiddenMarkovModelTagger, DictionaryConditionalProbDist, DictionaryProbDist

def prob(**kwargs):
    return DictionaryProbDist(kwargs)
def condprob(**kwargs):
    return DictionaryConditionalProbDist(kwargs)

hmm = HiddenMarkovModelTagger(
    symbols = ["sound", "sounds", "nice", "dot"],
    states = ["ADJ", "N", "V", "END"],
    transitions = condprob(
        ADJ = prob(N = 0.4, V = 0.4, END = 0.2),
        N = prob(ADJ = 0.2, V = 0.7, END = 0.1),
        V = prob(N = 0.5, END = 0.5),
    ),
    outputs = condprob(
        ADJ = prob(sound = 0.3, nice = 0.7),
        N = prob(sound = 0.5, nice = 0.5),
        V = prob(sound = 0.8, sounds = 0.2),
        END = prob(dot = 1.0),
    ),
    priors = prob(ADJ = 0.3, N = 0.4, V = 0.3, END = 0)
)

for words in ["nice sound dot", "sound sounds nice dot", "sound sound sound dot"]:
    tagged = hmm.tag(words.split())
    print "Best tags:", tagged
    print "Forward probability:", hmm.probability([(w, None) for w in words.split()])
    print "Sequence probability:", hmm.probability(tagged)
    print

#Best tags: [('nice', 'N'), ('sound', 'V'), ('dot', 'END')]
#Forward probability: 0.0962
#Sequence probability: 0.056
#
#Best tags: [('sound', 'N'), ('sounds', 'V'), ('nice', 'N'), ('dot', 'END')]
#Forward probability: 0.00088
#Sequence probability: 0.0007
#
#Best tags: [('sound', 'V'), ('sound', 'N'), ('sound', 'V'), ('dot', 'END')]
#Forward probability: 0.028456
#Sequence probability: 0.0168

