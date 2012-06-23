import os
import itertools
import time
import unicodedata
import cPickle as pickle
from natlist import nationalities
from professions import professions
from cStringIO import StringIO


class Token(object):
    __slots__ = ["word", "part"]
    def __init__(self, word, part):
        self.word = word
        self.part = part
    def __repr__(self):
        return "<%s/%s>" % (self.word, self.part)


def strip_accents(string):
    return str(unicodedata.normalize('NFKD', unicode(string)).encode('ASCII', 'ignore'))

def read_line(f):
    line = f.readline()
    if not line:
        raise EOFError()
    return strip_accents(line.strip().decode("utf8"))

def build_topics(f):
    topics = {}
    try:
        while True:
            title = read_line(f)
            if not title:
                continue
            text = ""
            while True:
                line = read_line(f)
                if not line:
                    break
                text += line
            topics[title] = tuple((lambda (w, p): (w.lower(), p))(word.split("/", 1)) 
                for word in text.split() if "/" in word)
    except EOFError:
        pass
    return topics

def find_people(topics):
    people = {}
    for title, tokens in topics.iteritems():
        words = [t[0] for t in tokens]
        first_sentence = list(itertools.takewhile(lambda w: w != ".", words))
        if "born" in first_sentence or "died" in first_sentence:
            people[title] = tokens
        elif "-lrb-" in first_sentence and "-rrb-" in first_sentence:
            lrb = words.index("-lrb-")
            rrb = words.index("-rrb-")
            span_tokens = tokens[lrb+1:rrb]
            for word, part in span_tokens:
                if part == "CD" and word.isdigit() and len(word) == 4: # a year
                    people[title] = tokens
                    break
    return people

def build_sentences(tokens):
    tokens2 = []
    in_quote = False
    tokens = list(tokens)
    while tokens:
        w, p = tokens.pop(0)
        if (w, p) in (("''", "''"), ("``", "``")):
            w, p = '"', '"'
        if p == '"':
            words = []
            while tokens:
                w, p = tokens.pop(0)
                if (w, p) in (("''", "''"), ("``", "``")):
                    w, p = '"', '"'
                if p == '"':
                    break
                words.append(w)
            tokens2.append(('"' + " ".join(words) + '"', "NNP"))
            continue

        if not w.strip():
            continue
        if tokens2 and tokens2[-1][1] == p and p in ("NN", "NNS", "NNP"):
            tokens2[-1] = (tokens2[-1][0] + " " + w, p)
        elif tokens2 and tokens and p == "CC" and tokens[0][1] == tokens2[-1][1]:
            tokens2[-1] = (tokens2[-1][0] + " " + w, tokens2[-1][1])
        else:
            tokens2.append((w, p))
    sentences = [[]]
    for w, p in tokens2:
        if p == ".":
            sentences.append([])
        else:
            sentences[-1].append(Token(w, p))
    if not sentences[-1]:
        del sentences[-1]
    return sentences

months = {
    "jan" : 1,
    "january" : 1,  
    "feb" : 2,
    "february" : 2,
    "march" : 3,
    "mar" : 3,
    "april" : 4,
    "apr" : 4,
    "may" : 5,
    "june" : 6,
    "jun" : 6,
    "july" : 7,
    "jul" : 7,
    "august" : 8,
    "aug" : 8,
    "september" : 9,
    "sep" : 9,
    "october" : 10,
    "oct" : 10,
    "november" : 11,
    "nov" : 11,
    "december" : 12,
    "dec" : 12,
}

numbers = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", 
    "eleven", "twelve", "thirteen", "forteen", "fifteen", "sixteen", "seventeen", "eighteen", 
    "ninteen", "twenty"]

def extract_date(sentence):
    y = None
    m = None
    d = None
    while sentence:
        tok = sentence.pop(0)
        if tok.part == "CD" and tok.word[-2:] in ("st", "nd", "rd", "th"):
            tok.word = tok.word[:-2]
        if m is None and tok.word.lower() in months:
            m = months[tok.word.lower()]
        elif m is not None and tok.part == "CD" and len(tok.word) <= 2:
            try:
                d = int(tok.word)
            except ValueError:
                d = numbers.index(tok.word.lower())
        elif tok.part == "CD" and len(tok.word) == 4 and (tok.word.isalpha() or tok.word.isdigit()):
            try:
                y = int(tok.word)
            except ValueError:
                y = numbers.index(tok.word.lower())
            break
    if (y, m, d) == (None, None, None):
        return None
    return (y, m, d)

class Person(object):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        attrs = ", ".join("%s = %r" % (k, v) for k, v in self.__dict__.items() if k != "name") 
        return "Person(%r, %s)" % (self.name, attrs)

def extract_lifespan(sentences):
    first_sentence = list(sentences[0])
    born = extract_date(first_sentence)
    died = extract_date(first_sentence)
    return born, died

non_occupations = set(["mother", "father", "number", "brother", "sister", "classical"])

def extract_occupation(sentences):
    properties = []
    for sent in sentences:
        properties.extend(tok.word for tok in 
            itertools.dropwhile(lambda tok: tok.part not in ("VBZ", "VBD"), sent) 
            if tok.part in ("NN", "JJ") and tok.word not in non_occupations and 
                (tok.word in professions or any(
                    tok.word.endswith(suffix) for suffix in ["er", "or", "ess"])
                )
            )
    return properties

def extract_achievements(sentences):
    properties = []
    for sent in sentences:
        for i, tok in enumerate(sent):
            if tok.word in ("was", "were", "is", "are") and i < len(sent) - 1 and sent[i+1].word in ("a", "an", "one", "the"):
                properties.append(" ".join(tok.word for tok in 
                    itertools.takewhile(lambda tok: tok.part != ",", sent[i+1:])))
        for i, tok in enumerate(sent):
            if tok.word in ("won", "discovered", "invented", "recognized", "achieved", "held", "famous"):
                properties.append(" ".join(tok.word for tok in 
                    itertools.takewhile(lambda tok: tok.part != ",", sent[i:])))
        for i, tok in enumerate(sent):
            if tok.word == "known" and i < len(sent) - 1 and sent[i + 1].word in ("as", "for"):
                properties.append(" ".join(tok.word for tok in 
                    itertools.takewhile(lambda tok: tok.part != ",", sent[i+2:])))
    return properties

def extract_nationality(sentences):
    suspects = []
    for sent in sentences[0:3]:
        suspects.extend(tok.word for tok in 
            itertools.dropwhile(lambda tok: tok.part not in ("VBZ", "VBD"), sent)
            if tok.part in ("JJ", "NNP") and tok.word.lower() in nationalities)
    if suspects:
        return suspects[0]
    else:
        return None

def extract_other_names(sentences):
    other_names = []
    for tok in sentences[0]:
        if tok.part == "NNP" and " " in tok.word:
            other_names.append(tok.word)
    for sent in sentences:
        seen_nick = seen_verb = seen_is = False
        for tok in sent:
            if tok.word in ("name", "nickname"):
                seen_nick = True
            elif tok.part in ("VBZ", "VBD"):
                seen_is = True
                if seen_nick:
                    seen_verb = True
            elif seen_is and tok.word in ("referred", "named", "nicknamed"):
                seen_verb = True
            elif seen_verb and tok.part == "NNP":
                other_names.append(tok.word)
            
    return other_names

def extract_place_of_birth(sentences):
    for sent in sentences[0:3]:
        for i in range(len(sent) - 1):
            #if sent[i].word in ("born", "raised", "grew", "lived"):
                if sent[i].word in ("in", "at"):
                    if sent[i+1].part == "NNP":
                        return sent[i + 1].word
    return None

def extract_spouse(sentences):
    for sent in sentences:
        for i in range(len(sent) - 1):
            if sent[i].word in ("married", "wedded", "spouse"):
                for j in range(i + 1, len(sent)):
                    if sent[j].part == "NNP":
                        return sent[j].word
    return None

def extract_info(people, title):
    sentences = build_sentences(people[title])
    entity = Person(title)
    entity.born, entity.died = extract_lifespan(sentences)
    entity.nationality = extract_nationality(sentences)
    entity.place_of_birth = extract_place_of_birth(sentences)
    entity.spouse = extract_spouse(sentences)
    entity.occupation = set(extract_occupation(sentences))
    entity.other_names = set(extract_other_names(sentences)) - set([title])
    entity.recognized_for = set(extract_achievements(sentences))
    return entity

def build_ontology(f):
    if isinstance(f, str):
        f = open(f, "r")
    people = find_people(build_topics(f))
    ontology = {}
    for k in sorted(people):
        ontology[k] = extract_info(people, k)
    return people, ontology

class OnotologyQuery(object):
    def __init__(self, ontology):
        self.ontology = ontology
    def what_is_the_occupation_of(self, name):
        return self.ontology[name].occupation
    def when_was_born(self, name):
        return self.ontology[name].born
    def is_alive(self, name):
        return self.ontology[name].died is None
    def find_two_X_born_on_same_year(self, occupation):
        same_year = {}
        for name, info in self.ontology.iteritems():
            if occupation.lower() in info.occupation and info.born is not None:
                year = info.born[0]
                if year in same_year:
                    return same_year[year], name
                else:
                    same_year[year] = name

def time_table(filename):
    with open(filename, "r") as f:
        lines = f.read().splitlines()
    chunk_size = len(lines) / 10

    print "  file size  |    %   | Time | Num of People"
    print "=============+========+======+=============="

    for i in range(1, 11):
        chunk = "\n".join(lines[:chunk_size * i])
        t0 = time.time()
        people, ontology = build_ontology(StringIO(chunk))
        t1 = time.time()
        print "%12s | %5s%% | % .2f | %s" % (len(chunk), i*10, t1-t0, len(people))
        #print "%s\t%s\t%s" % (len(chunk) / (1024 * 1024), t1-t0, len(people))


if __name__ == "__main__":
    #time_table("full_pos.txt")
    
    people, ontology = build_ontology("full_pos.txt")
    pickle.dump(ontology, open("onto.pkl", "wb"), 2)
    pickle.dump(people, open("people.pkl", "wb"), 2)
    #ontology = pickle.load(open("onto.pkl", "rb"))
    #people = pickle.load(open("people.pkl", "rb"))
    
    pickle.dump(ontology, open("onto.pkl", "wb"), 2)
    ontology = pickle.load(open("onto.pkl", "rb"))

    print ontology["Elvis Costello"]
    print ontology["Yves Chauvin"]
    print ontology["Marilyn Monroe"]
    print ontology["Elvis Presley"]
    
    q = OnotologyQuery(ontology)
    print q.what_is_the_occupation_of("Marilyn Monroe")
    print q.what_is_the_occupation_of("Yves Chauvin")
    print q.when_was_born("Marilyn Monroe")
    print q.is_alive("Marilyn Monroe")
    print q.is_alive("Johnny Cash")
    
    print q.find_two_X_born_on_same_year("actor")
    p1, p2 = q.find_two_X_born_on_same_year("musician")
    print (p1, p2)
    
    print ontology[p1]
    print ontology[p2]


