from nltk.tag.hmm import *
symbols = ['up', 'down', 'unchanged']
states = ['bull', 'bear', 'static']

def probdist(values, samples):
    d = {}
    for value, item in zip(values, samples):
        d[item] = value
    return DictionaryProbDist(d)

def conditionalprobdist(array, conditions, samples):
    d = {}
    for values, condition in zip(array, conditions):
        d[condition] = probdist(values, samples)
    return DictionaryConditionalProbDist(d)

A = array([[0.6, 0.2, 0.2], [0.5, 0.3, 0.2], [0.4, 0.1, 0.5]], float64)
A = conditionalprobdist(A, states, states)
#B = array([[0.7, 0.1, 0.2], [0.1, 0.6, 0.3], [0.3, 0.3, 0.4]], float64)
#B = conditionalprobdist(B, states, symbols)
#pi = array([0.5, 0.2, 0.3], float64)
#pi = probdist(pi, states)
#model = HiddenMarkovModelTagger(symbols=symbols, states=states,
#                                transitions=A, outputs=B, priors=pi)

print A["bear"].samples

