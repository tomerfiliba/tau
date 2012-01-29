class State(object):
    def __init__(self, name):
        self.name = name
        self.a_table = {}
        self.b_table = {}
    def __repr__(self):
        return self.name
    def connect(self, state, prob):
        self.a_table[state] = prob
    def get_a(self, state):
        return self.a_table.get(state, 0.0)
    def get_b(self, ch):
        return self.b_table.get(ch, 0.0)
    def set_b(self, ch, prob):
        self.b_table[ch] = prob

class HMM(object):
    def __init__(self, num_of_internal_states):
        self.num_of_internal_states = num_of_internal_states
        self.q0 = State("q0")
        self.qF = State("qF")
        self.states = {"q0" : self.q0, 0 : self.q0, 
            "qF" : self.qF, num_of_internal_states + 1 : self.qF}
        for i in range(1, num_of_internal_states + 1):
            self.states["q%d" % (i,)] = self.states[i] = State("q%d" % (i,))
    def __getitem__(self, key):
        return self.states[key]
    def internal_states(self):
        return (self.states[k] for k in range(1, self.num_of_internal_states+1))
    def all_states(self):
        return (self.states[k] for k in range(0, self.num_of_internal_states+2))

class TableCell(object):
    def __init__(self):
        self.value = 0.0
        self.back = None
    def __repr__(self):
        return "(%r, %r)" % (self.value, self.back)

class Table(object):
    def __init__(self, hmm, observations):
        self.hmm = hmm
        self.cells = {s : [TableCell() for i in range(len(observations))] 
            for s in hmm.all_states()}
    def __str__(self):
        lines = []
        for s in self.hmm.all_states():
            row = self.cells[s]
            line = " | ".join("%.5f %04s" % (cell.value, cell.back) for cell in row)
            lines.append(repr(s) + " | " + line)
        return "\n".join(lines)
    def __getitem__(self, ind):
        s, t = ind
        return self.cells[s][t]

def viterbi(hmm, observations):
    table = Table(hmm, observations)
    
    for s in hmm.internal_states():
        table[s, 0].value = hmm.q0.get_a(s) * s.get_b(observations[0])
        table[s, 0].back = hmm.q0
    
    for t, ch in enumerate(observations[1:]):
        for s in hmm.internal_states():
            maxv2 = -1
            for q in hmm.internal_states():
                v1 = table[q, t].value * q.get_a(s) * s.get_b(ch)
                v2 = table[q, t].value * q.get_a(s)
                if v1 > table[s, t+1].value:
                    table[s, t+1].value = v1
                if v2 > maxv2:
                    maxv2 = v2
                    table[s, t+1].back = q

    t = len(observations) - 1
    for q in hmm.internal_states():
        v = table[q, t].value * q.get_a(hmm.qF)
        if v > table[hmm.qF, t].value:
            table[hmm.qF, t].value = v
            table[hmm.qF, t].back = q

    print table
    
    s = hmm.qF
    path = []
    #t -= 1
    while t >= 0:
        path.append(s)
        s = table[s, t].back
        t -= 1
    path.append(s)
    path.append(hmm.q0)
    return path[::-1]
    
    


if __name__ == "__main__":
    h = HMM(2)
    # q0
    h["q0"].connect(h["q1"], 0.7)
    h["q0"].connect(h["q2"], 0.3)
    # q1
    h["q1"].set_b("u", 0.5)
    h["q1"].set_b("v", 0.5)
    h["q1"].connect(h["q1"], 0.5)
    h["q1"].connect(h["q2"], 0.3)
    h["q1"].connect(h["qF"], 0.2)
    # q2
    h["q2"].set_b("u", 0.8)
    h["q2"].set_b("v", 0.2)
    h["q2"].connect(h["q1"], 0.4)
    h["q2"].connect(h["q2"], 0.5)
    h["q2"].connect(h["qF"], 0.1)
    
    #print viterbi(h, "vvuvuu")
    #print viterbi(h, "uuuvuuv")
    print viterbi(h, "u")








