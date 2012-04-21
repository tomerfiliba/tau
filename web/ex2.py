import os
import re
import itertools
import random
from math import sqrt


pattern = re.compile('<a +href *= *" *(.*?) *" *>')
strip_html = re.compile('</{0,1}.*?>')

class Node(object):
    def __init__(self, url, out_links, words):
        self.url = url
        self.out_links = out_links
        self.in_links = []
        self.words = words
    def __repr__(self):
        return "<Node %r>" % (self.url,)

#===================================================================================================
# Question 1
#
# The function build_graph takes a directory and builds a graph (dictionary) 
# of Node objects. Each node contains a list of outgoing links, the list of words  
# that make up the document's data (lower-case and stripped of HTML), and a list
# of incoming-links (by reversing the graph)
#
#===================================================================================================
def build_graph(directory):
    graph = {}
    
    # build graph
    for fn in os.listdir(directory):
        data = open(os.path.join(directory, fn), "r").read()
        links = pattern.findall(data)
        assert data.count("<") % 2 == 0 and data.count("<") // 2 == len(links)
        data2 = strip_html.sub("", data)
        assert "<" not in data2 and ">" not in data2
        words = data2.lower().split()
        graph[fn] = Node(fn, links, words)
    
    # calculate back-links for each node
    for p in graph.values():
        for q in graph.values():
            if p.url in q.out_links:
                p.in_links.append(q.url)
    
    return graph

#===================================================================================================
# Question 2
#
# This is an implementation of the HITS algorithm for our data structure (as 
# defined in Q1). It takes a graph and performs HITS iterations until the results
# converge. We defined convergence as the difference between the old two norms 
# and the current ones being less than a certain epsilon, e.g.
# 
#   |old_hubness_norm - curr_hubness_norm| < epsilon and
#   |old_authority_norm - curr_authority_norm| < epsilon
#
# The epsilon we used is 0.0001. The function also accepts two optional arguments,
# hub_init_func and auth_init_func, which are zero-argument functions that return
# the initial values for the hubness and authority (respectively) of each node.
# By default, these functions just return 1, but you can pass a different function, 
# such as random.random, which would initialize each node with random values in the
# range (0,1).
#
# A note on normalization: without normalization, the values will not converge, 
# because each value is the sum of non-negative values. Without normalization, 
# the program crashed with an overflow error in just 143 iterations. The numbers just 
# kept growing and growing, and naturally the values did not converge. Therefore, 
# normalizations after each iteration is necessary.
#
# For the graph built in Q1, and random init values, the function takes 6-8 
# iterations to converge (in ~20 trials). The final values are
#
# site                 | hub       | auth       
# =====================+===========+============
# latimes.txt          | 0.2062368 | 0.0000000
# haaretz.txt          | 0.2755813 | 0.0000000
# webmd.txt            | 0.3438239 | 0.1099133
# nytimes.txt          | 0.4848676 | 0.0989221
# yahoo.txt            | 0.3088561 | 0.3529728
# nydailynews.txt      | 0.0281863 | 0.0898502
# cnn.txt              | 0.0000000 | 0.7238039
# guardian.txt         | 0.4443981 | 0.1420309
# aol.txt              | 0.3751420 | 0.2612281
# bbc.txt              | 0.3062712 | 0.4828723
#
# So the top 3 sites in terms of hubness are: aol.txt, guardian.txt, nytimes.txt
# And in terms of authority: yahoo.txt, bbc.txt, cnn.txt
#
#===================================================================================================
def calculate_HITS(graph, hub_init_func = lambda: 1, auth_init_func = lambda: 1, 
        epsilon = 0.0001):
    
    for p in graph.values():
        p.hub = hub_init_func()
        p.auth = auth_init_func()
    
    prev_norm1 = 0
    prev_norm2 = 0
    for i in itertools.count(1):
        # update all authority values first
        for p in graph.values():
            p.auth = 0
            p.auth = sum(graph[q].hub for q in p.in_links)
        
        # normalize auth values
        norm1 = sqrt(sum(p.auth ** 2 for p in graph.values()))
        for p in graph.values():
            p.auth = p.auth / norm1
        
        # update all hub values
        for p in graph.values():
            p.hub = 0
            p.hub = sum(graph[q].auth for q in p.out_links)
        
        # normalize auth values
        norm2 = sqrt(sum(p.hub ** 2 for p in graph.values()))
        for p in graph.values():
            p.hub = p.hub / norm2
        
        # test for convergence - if the two norms are the same as the 
        # previous norms (up to epsilon)
        if abs(norm1 - prev_norm1) < epsilon and abs(norm2 - prev_norm2) < epsilon:
            break
        prev_norm1 = norm1
        prev_norm2 = norm2
    
    return i

def show_HITS(graph):
    print "site                 | hub       | auth       "
    print "=====================+===========+============"
    for p in g.values():
        print "%-20s | %.7f | %.7f" % (p.url, p.hub, p.auth)

#===================================================================================================
# Question 3
#===================================================================================================
def term_freq(word, node):
    return node.words.count(word.lower())


if __name__ == "__main__":
    g = build_graph("ex2input")
    iterations_until_convergence = calculate_HITS(g) #, random.random, random.random)
    print iterations_until_convergence
    show_HITS(g)
    print sorted(g.values(), key = lambda n: n.hub)[-3:]
    print sorted(g.values(), key = lambda n: n.auth)[-3:]


