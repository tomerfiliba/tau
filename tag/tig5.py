#==============================================================================
#==============================================================================
# Tree Insertion Grammar recognizer and parser
# By Tomer Filiba, 2012-04-28
# Based on an algorithm outlined in [Schabes 94]
#==============================================================================
#==============================================================================

# Whether or not to use unicode (some terminals choke on this)
USE_UNICODE = True
if USE_UNICODE:
    SYM_DOWN_ARROW = u"\u2193"
    SYM_RIGHT_ARRROW = u"\u2192"
    SYM_DOT = u"\u00b7"
else:
    SYM_DOWN_ARROW = "^"
    SYM_RIGHT_ARRROW = "->"
    SYM_DOT = "@"

#==============================================================================
# Exceptions
#==============================================================================
class GrammarError(Exception):
    """Raised when constructing the TIG"""
    pass
class ParsingError(Exception):
    """Raised when parsing fails"""
    pass

#==============================================================================
# Building Blocks
#==============================================================================
class NonTerminal(object):
    """
    Represents a non-terminal in the grammar; there's no need for a class to 
    represent terminals, as we just use strings for that
    """
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return self.name
    def __call__(self, *children):
        return Tree(self, children)

class Foot(object):
    """
    Represents a foot node in an auxiliary tree; it simply wraps the given 
    non-terminal
    """
    def __init__(self, nonterminal):
        self.nonterminal = nonterminal
    def __str__(self):
        return "%s*" % (self.nonterminal,)

class Tree(object):
    """
    Represents a grammar tree. This could be either an initial, left-aux or
    right-aux tree, or a deviation tree (we reuse this class for this purpose 
    too). Note that trees compare by *value*, not identity, and are 
    immutable once created.
    """
    INIT_TREE = 0
    LEFT_AUX = 1
    RIGHT_AUX = 2
    DERIVATION_TREE = 3
    
    def __init__(self, root, children, type = INIT_TREE):
        self.root = root
        self.children = tuple(children)
        self.type = type
        self._hash = None
        self._path = NotImplemented

    def __str__(self):
        return "%s(%s)" % (self.root, 
            ", ".join((repr(c) if isinstance(c, str) else str(c)) 
                for c in self.children))
    def __eq__(self, other):
        if not isinstance(other, Tree):
            return False
        return (self.root, self.children) == (other.root, other.children)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        if self._hash is None:
            self._hash = hash((self.root, self.children))
        return self._hash
    
    def _path_to_foot(self):
        for i, child in enumerate(self.children):
            if isinstance(child, Foot):
                return [i]
            elif isinstance(child, Tree):
                p = child._path_to_foot()
                if p:
                    return [i] + p
        return None
    
    def path_to_foot(self):
        """
        Returns the path from the root of this tree to the foot, or None
        if no foot exists; this is similar to Gorn tree-addresses
        """
        if self._path is NotImplemented:
            self._path = self._path_to_foot()
        return self._path
    
    def _substitute(self, i, child):
        children2 = list(self.children)
        children2[i] = child
        return Tree(self.root, children2, self.DERIVATION_TREE)
    
    def deep_substitute(self, i, child):
        """
        Returns a copy of this tree, in which the i'th child is replaced by 
        the given subtree. If this tree contains a foot, substitution occurs 
        in the i'th child of the parent of the foot instead
        """
        path = self.path_to_foot()
        if not path:
            return self._substitute(i, child)
        return self._deep_substitute(path[:-1], i, child)
    
    def _deep_substitute(self, path, i, child):
        if not path:
            return self._substitute(i, child)
        children2 = list(self.children)
        children2[path[0]]._deep_substitute(path[1:], i, child)
        return Tree(self.root, children2, self.DERIVATION_TREE)
    
    def substitute_foot(self, subtree):
        """
        Returns a copy of this tree, in which the foot (should it exist) is 
        replaced by the given subtree
        """
        children2 = list(self.children)
        for i, child in enumerate(children2):
            if isinstance(child, Foot):
                children2[i] = subtree
                break
            elif isinstance(child, Tree):
                children2[i] = child.substitute_foot(subtree)
        return Tree(self.root, children2, self.DERIVATION_TREE)
    
    def leaves(self):
        """
        returns an iterator over the non-empty leaves of this tree
        """
        for c in self.children:
            if isinstance(c, Tree):
                for n in c.leaves():
                    yield n
            elif c == "": # ignore epsilon productions in leaves:
                pass
            else:
                yield c
    
    def show(self, level = 0):
        """
        Prints the tree in a human-readable manner
        """
        print "   " * level + str(self.root)
        for c in self.children:
            if isinstance(c, Tree):
                c.show(level + 1)
            else:
                print "   " * (level + 1) + str(c)

class TIG(object):
    """
    Represents a TIG grammar instance; it basically holds the initial and
    auxiliary trees that make up the grammar. Upon creation, this class 
    verifies the given trees indeed form a valid TIG.  
    """
    def __init__(self, init_trees, aux_trees):
        self.init_trees_by_symbol = {}
        for t in init_trees:
            leaves = list(t.leaves())
            if not leaves:
                raise GrammarError("Initial trees must not be empty", t)
            if any(isinstance(n, Foot) for n in leaves):
                raise GrammarError("Initial trees cannot contain foot leaves")
            if t.root not in self.init_trees_by_symbol:
                self.init_trees_by_symbol[t.root] = []
            self.init_trees_by_symbol[t.root].append(t)

        self.left_aux_trees_by_symbol = {}
        self.right_aux_trees_by_symbol = {}
        for t in aux_trees:
            leaves = list(t.leaves())
            if not leaves:
                raise GrammarError("Auxiliary trees must not be empty", t)
            if len([n for n in leaves if isinstance(n, Foot)]) != 1:
                raise GrammarError("Auxiliary trees must contain exactly one "
                    "foot", t)
            if isinstance(leaves[-1], Foot):
                foot = leaves[-1]
                coll = self.left_aux_trees_by_symbol
                t.type = Tree.LEFT_AUX
            elif isinstance(leaves[0], Foot):
                foot = leaves[0]
                coll = self.right_aux_trees_by_symbol
                t.type = Tree.RIGHT_AUX
            else:
                raise GrammarError("Auxiliary trees must contain either a "
                    "leftmost or a rightmost foot", t)
            if foot.nonterminal != t.root:
                raise GrammarError("The foot of an auxiliary tree must be "
                    "of the same nonterminal as the root", t)
            
            if t.root not in coll:
                coll[t.root] = []
            coll[t.root].append(t)
    
    def get_init_trees_for(self, symbol):
        """
        Returns a (possibly empty) list of initial trees whose roots are the 
        given non-terminal
        """
        return self.init_trees_by_symbol.get(symbol, ())
    def get_left_aux_trees_for(self, symbol):
        """
        Returns a (possibly empty) list of left-auxiliary trees whose roots 
        are the given non-terminal
        """
        return self.left_aux_trees_by_symbol.get(symbol, ())
    def get_right_aux_trees_for(self, symbol):
        """
        Returns a (possibly empty) list of right-auxiliary trees whose roots
        are the given non-terminal
        """
        return self.right_aux_trees_by_symbol.get(symbol, ())

#==============================================================================
# Chart and Chart States
#==============================================================================
class State(object):
    """
    The chart state. This is in essence a 4-tuple <tree, dot, i, j>, with some 
    helper methods
    """
    def __init__(self, tree, dot, i, j):
        self.tree = tree
        self.dot = dot
        self.i = i
        self.j = j
        self.index = None
        self._hash = None
    def __str__(self):
        prod = ["%s%s" % (c, SYM_DOWN_ARROW) 
            if isinstance(c, NonTerminal) else str(c) 
            for c in self.tree.children]
        prod.insert(self.dot, SYM_DOT)
        return "%s %s %s,  %r:%r" % (self.tree.root, SYM_RIGHT_ARRROW, 
            " ".join(prod), self.i, self.j)
    def __eq__(self, other):
        return (self.tree, self.dot, self.i, self.j) == (
                other.tree, other.dot, other.i, other.j)
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        if self._hash is None:
            self._hash = hash((self.tree, self.dot, self.i, self.j))
        return self._hash
    def is_complete(self):
        """
        Returns True iff the dot is passed the last child (thus the state 
        is complete)
        """
        return self.dot >= len(self.tree.children)
    def next(self):
        """
        Return the next (first-level only) production of this tree, 
        or None if we've reached the end
        """
        if self.is_complete():
            return None
        return self.tree.children[self.dot]

class ChartItem(object):
    """
    A helper object, associated with each chart state, that holds the reasons
    for adding this state and the state's subtree builders
    """
    UNPROCESSED = 1
    PROCESSING = 2
    PROCESSED = 3
    
    def __init__(self, reason, subtreefunc):
        self.reasons = {reason}
        self.subtreefuncs = {subtreefunc}
        self.subtrees = set()
        # `stage` serves as a marker for get_subtrees()
        self.stage = self.UNPROCESSED
    
    def add(self, reason, subtreefunc):
        """
        Adds a reason and a subtree-builder to this chart item
        """
        self.reasons.add(reason)
        self.subtreefuncs.add(subtreefunc)

class Chart(object):
    """
    Represents the parser chart. It comprises of states (without duplicates),
    but it preserves the ordering relations for debugging purposes. With each
    state we associates a ChartItem, to hold some extra info. 
    States are add()ed to the chart, but they don't actually become part of 
    it until commit()ted. This prevents some issues with dictionary iteration.
    """
    
    def __init__(self):
        self._states = {}
        self._ordered_states = []
        self._changes = []
    def __iter__(self):
        return iter(self._ordered_states)
    def __len__(self):
        return len(self._ordered_states)
    def __getitem__(self, index):
        return self._ordered_states[index]
    
    def add(self, state, reason, subtreefunc = None, *args):
        """
        Adds a new state to the chart, including the state's reason and 
        subtree-builder. Note that it's not actually added to the chart 
        until commit() is called
        """
        if subtreefunc is None:
            subtreefunc = BUILD_CONST
            args = (state.tree,)
        self._changes.append((state, reason, (subtreefunc, args)))
    
    def commit(self):
        """
        commits the changes to the chart -- returns True if the chart
        has grew, False otherwise
        """
        added = False
        while self._changes:
            st, reason, subtreefunc = self._changes.pop(0)
            if st not in self._states:
                st.index = len(self._ordered_states)
                self._ordered_states.append(st)
                self._states[st] = ChartItem(reason, subtreefunc)
                added = True
            else:
                self._states[st].add(reason, subtreefunc)

        return added

    def get_subtrees(self, st):
        """
        Gets the set of subtrees for a given state; this is memoized (cached) 
        so once the subtrees of some state have been built, 
        future calls are O(1)
        """
        item = self._states[st]
        if item.stage == ChartItem.PROCESSED:
            return item.subtrees
        # make sure we're not accidentally reentrant
        assert item.stage == ChartItem.UNPROCESSED
        item.stage = ChartItem.PROCESSING
        for func, args in item.subtreefuncs:
            item.subtrees.update(func(self, *args))
        item.stage = ChartItem.PROCESSED
        return item.subtrees
    
    def show(self, only_completed = False):
        """
        Print the chart in a human-readable manner
        """
        for st in self._ordered_states:
            if only_completed and not st.is_complete():
                continue
            print "%3d | %-40s | %s" % (st.index, st, 
                " ; ".join(self._states[st].reasons))
        print "-" * 80

#==============================================================================
# Tree extraction combinators: 
#
# Whenever we add a new state to the chart, we associate with it a 
# subtree-builder, which serves us later (we get_subtrees() is called). 
# These builders combine partial trees to form bigger ones, according to 
# the rules of the grammar 
#==============================================================================
def BUILD_CONST(chart, t):
    return [t]

def BUILD_PROPAGATE(chart, st):
    return chart.get_subtrees(st)

def BUILD_SUBSTITUTION(chart, st, st2):
    return [t1.deep_substitute(st.dot, t2) 
        for t1 in chart.get_subtrees(st) for t2 in chart.get_subtrees(st2)]

def BUILD_AUX(chart, st, st2):
    return [t2.substitute_foot(t1) 
        for t1 in chart.get_subtrees(st) for t2 in chart.get_subtrees(st2)]

#==============================================================================
# Parser
#==============================================================================
def handle_left_adj(grammar, chart, st):
    """
    handles the case of left-adjunction rules (2) and (3)
    """
    if st.dot != 0:
        return
    
    # (2)
    for t in grammar.get_left_aux_trees_for(st.tree.root):
        chart.add(State(t, 0, st.j, st.j), "[2]/%d" % (st.index,))
    
    # (3)
    for st2 in chart:
        if (st2.tree.type == Tree.LEFT_AUX and st.tree.root == st2.tree.root 
                and st.j == st2.i and st2.is_complete()): 
            chart.add(State(st.tree, 0, st.i, st2.j), 
                "[3]/%d,%d" % (st.index, st2.index), 
                BUILD_AUX, st, st2)

def handle_scan(grammar, chart, st, token):
    """
    handles the case of scanning rules (4), (5) and (6)
    """
    prod = st.next()
    if isinstance(prod, str):
        if prod == token:
            # (4)
            chart.add(State(st.tree, st.dot+1, st.i, st.j+1), 
                "[4]/%d" % (st.index,), 
                BUILD_PROPAGATE, st)
        elif prod == "":
            # (5)
            chart.add(State(st.tree, st.dot+1, st.i, st.j), 
                "[5]/%d" % (st.index,), 
                BUILD_PROPAGATE, st)
    elif isinstance(prod, Foot):
        # (6)
        chart.add(State(st.tree, st.dot+1, st.i, st.j), 
            "[6]/%d" % (st.index,), 
            BUILD_PROPAGATE, st)

def handle_substitution(grammar, chart, st):
    """
    handles the case of substitution rules (7) and (8)
    """
    prod = st.next()
    if isinstance(prod, NonTerminal):
        # (7)
        for t in grammar.get_init_trees_for(prod):
            chart.add(State(t, 0, st.j, st.j), "[7]/%d" % (st.index,))
        
        # (8)
        for st2 in chart:
            if (st2.tree.root == prod and st.j == st2.i and st2.is_complete() 
                    and st2.tree.type == Tree.INIT_TREE):
                chart.add(State(st.tree, st.dot + 1, st.i, st2.j), 
                    "[8]/%d,%d" % (st.index, st2.index), 
                    BUILD_SUBSTITUTION, st, st2)
 
def handle_subtree_traversal(grammar, chart, st):
    """
    handles the case of subtree-traversal rules (9) and (10)
    """
    prod = st.next()
    if isinstance(prod, Tree):
        # (9)
        chart.add(State(prod, 0, st.j, st.j), "[9]/%d" % (st.index,)) 
        
        # (10)
        for st2 in chart:
            if st2.tree == prod and st.j == st2.i and st2.is_complete():
                chart.add(State(st.tree, st.dot + 1, st.i, st2.j), 
                    "[10]/%d,%d" % (st.index, st2.index), 
                    BUILD_SUBSTITUTION, st, st2)

def handle_right_adj(grammar, chart, st):
    """
    handles the case of right-adjunction rules (11) and (12)
    """
    if not st.is_complete():
        return
    
    # (11)
    for t in grammar.get_right_aux_trees_for(st.tree.root):
        chart.add(State(t, 0, st.j, st.j), "[11]/%d" % (st.index,))
    
    # (12)
    for st2 in chart:
        if (st2.tree.type == Tree.RIGHT_AUX and st2.tree.root == st.tree.root 
                and st.j == st2.i and st2.is_complete()):
            chart.add(State(st.tree, len(st.tree.children), st.i, st2.j), 
                "[12]/%d,%d" % (st.index, st2.index), BUILD_AUX, st, st2)

def parse(grammar, start_symbol, tokens, debug = False):
    """
    The actual parser: it takes a TIG grammar object, a start symbol 
    (NonTerminal) of that grammar, and a list of tokens, and returns 
    (hopefully) all possible parse trees for them.
    
    It works by first applying the initialization rule (1), 
    then applying rules (2)-(12) for as long as the chart keeps changing, 
    and once it's stable, it looks for matching states according to 
    acceptance rule (13).
    
    It then takes all matching states (normally there should be only one),
    extracts the trees of each state, and returns a set of them.
    
    Note that TIG is assumed to be lexicalized, or at least finitely-ambiguous, 
    so we know the number of trees is bounded.
    
    The parsing is done in O(|G|^2 * n^3), as discussed in the paper, 
    and tree extraction is performed in amortized linear time, per each tree.
    """
    if isinstance(tokens, str):
        tokens = tokens.split()
    chart = Chart()
    tokens = list(tokens)
    padded_tokens = [None] + tokens
    
    # (1)
    for t in grammar.get_init_trees_for(start_symbol):
        chart.add(State(t, 0, 0, 0), "[1]")
    
    # main loop: run (2)-(12) until no more changes occur
    while True:
        for st in chart:
            handle_left_adj(grammar, chart, st)
            tok = padded_tokens[st.j+1] if st.j+1 < len(padded_tokens) else None
            handle_scan(grammar, chart, st, tok)
            handle_substitution(grammar, chart, st)
            handle_subtree_traversal(grammar, chart, st)
            handle_right_adj(grammar, chart, st)
        
        if not chart.commit():
            # no more changes, we're done
            break

    # (13)
    matches = [st for st in chart if st.is_complete() and st.i == 0 
        and st.j == len(tokens) and st.tree.root == start_symbol 
        and st.tree.type == Tree.INIT_TREE]
    if debug:
        chart.show()
        print "Matches:", [st.index for st in matches]
        print

    # fail if no matching state was found
    if not matches:
        raise ParsingError("Grammar does not derive the given sequence")
    
    # extract trees, drop ones that do not generate the correct token sequence
    trees = set(t for m in matches for t in chart.get_subtrees(m)
         if list(t.leaves()) == tokens)
    
    # and make sure we didn't lose all trees, for then it's our fault
    assert trees
    return trees


