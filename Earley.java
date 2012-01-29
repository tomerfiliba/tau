import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;


/*
 * Terminology
 * ===========
 * Consider the following context-free rule:
 *     
 *     X -> A B C | A hello
 * 
 * We say rule 'X' has two __productions__: "A B C" and "A hello".
 * Each production is made of __production terms__, which can be either 
 * __terminals__ (in our case, "hello") or __rules__ (non-terminals, such as "A", "B", and "C")
 * 
 */
public class Earley 
{
	/* 
	 * an abstract notion of the elements that can be placed within production
	 */
	public interface ProductionTerm
	{
	}
	
	/*
	 * Represents a terminal element in a production
	 */
	public static class Terminal implements ProductionTerm
	{
		public final String value;
		public Terminal(String value) {
			this.value = value;
		}
		@Override
		public String toString()
		{
			return value;
		}

		@Override
		public boolean equals(Object other) 
		{
			if (this == other) {
				return true;
			}
			if (other == null) {
				return false;
			}
			if (other instanceof String) {
				return value.equals((String)other);
			}
			else if (other instanceof Terminal) {
				return value.equals(((Terminal)other).value);
			}
			return false;
		}
	}
	
	/*
	 * Represents a production of the rule. 
	 */
	public static class Production implements Iterable<ProductionTerm>
	{
		public final List<ProductionTerm> terms;
		public final List<Rule> rules;
		
		public Production(ProductionTerm... terms)
		{
			this.terms = Arrays.asList(terms);
			this.rules = getRules();
		}
		public Production(Object... terms)
		{
			this.terms = new ArrayList<ProductionTerm>(terms.length);
			for (Object item : terms) {
				if (item instanceof String) {
					this.terms.add(new Terminal((String)item));
				}
				else if (item instanceof ProductionTerm) {
					this.terms.add((ProductionTerm)item);
				}
				else {
					throw new IllegalArgumentException("Term must be ProductionTerm or String, not " + item);
				}
			}
			this.rules = getRules();
		}
		
		public int size() {
			return terms.size();
		}
		public ProductionTerm get(int index) {
			return terms.get(index);
		}
		@Override
		public Iterator<ProductionTerm> iterator() {
			return terms.iterator();
		}
		
		private List<Rule> getRules()
		{
			ArrayList<Rule> rules = new ArrayList<Rule>();
			for (ProductionTerm term : terms) {
				if (term instanceof Rule) {
					rules.add((Rule)term);
				}
			}
			return rules;
		}
		
		@Override
		public boolean equals(Object other) 
		{
			if (this == other) {
				return true;
			}
			if (other == null || other.getClass() != getClass()) {
				return false;
			}
			return terms.equals(((Production)other).terms);
		}

		@Override
		public String toString()
		{
			String s = "";
			if (!terms.isEmpty()) {
				for (int i = 0; i < terms.size() - 1; i++) {
					ProductionTerm t = terms.get(i);
					if (t instanceof Rule) {
						s += ((Rule)t).name;
					}
					else {
						s += t;
					}
					s += " ";
				}
				ProductionTerm t = terms.get(terms.size() - 1);
				if (t instanceof Rule) {
					s += ((Rule)t).name;
				}
				else {
					s += t;
				}
			}
			return s;
		}
	}

	// Epsilon transition: an empty production
	public static final Production Epsilon = new Production();
	
	/*
	 * A CFG rule. Since CFG rules can be self-referential, more productions may be added
	 * to them after construction. For example:
	 * 
	 * Grammar:
	 * 	   SYM -> a 
	 * 	   OP -> + | -
	 *     EXPR -> SYM | EXPR OP EXPR 
	 * 
	 * In Java:
	 *     Rule SYM = new Rule("SYM", new Production("a"));
	 *     Rule OP = new Rule("OP", new Production("+"), new Production("-"));
	 *     Rule EXPR = new Rule("EXPR", new Production(SYM));
	 *     EXPR.add(new Production(EXPR, OP, EXPR));            // needs to reference EXPR
	 * 
	 */
	public static class Rule implements ProductionTerm, Iterable<Production>
	{
		public final String name;
		public final ArrayList<Production> productions;
		
		public Rule(String name, Production... productions) {
			this.name = name;
			this.productions = new ArrayList<Production>(Arrays.asList(productions));
		}
		
		public void add(Production... productions) {
			this.productions.addAll(Arrays.asList(productions));
		}

		public int size() {
			return productions.size();
		}
		public Production get(int index) {
			return productions.get(index);
		}
		@Override
		public Iterator<Production> iterator() {
			return productions.iterator();
		}

		@Override
		public boolean equals(Object other) 
		{
			if (this == other) {
				return true;
			}
			if (other == null || other.getClass() != getClass()) {
				return false;
			}
			Rule other2 = (Rule)other;
			return name.equals(other2.name) && productions.equals(other2.productions);
		}

		@Override
		public String toString() {
			String s = this.name + " -> ";
			if (!productions.isEmpty()) {
				for (int i = 0; i < productions.size() - 1; i++) {
					s += productions.get(i) + " | ";
				}
				s += productions.get(productions.size() - 1);
			}
			return s;
		}
	}
	
	/*
	 * Represents a state in the Earley parsing table. A state has a its rule's name,
	 * the rule's production, dot-location, and starting- and ending-column in the parsing
	 * table.
	 */
	public static class TableState
	{
		public final String name;
		public final Production production;
		public final int dotIndex;
		public final TableColumn startCol;
		public TableColumn endCol;
		
		public TableState(String name, Production production, int dotIndex, TableColumn startCol)
		{
			this.name = name;
			this.production = production;
			this.dotIndex = dotIndex;
			this.startCol = startCol;
			endCol = null;
		}
		
		public boolean isCompleted() {
			return dotIndex >= production.size();
		}
		
		public ProductionTerm getNextTerm() {
			if (isCompleted()) {
				return null;
			}
			return production.get(dotIndex);
		}
		
		@Override
		public boolean equals(Object other) 
		{
			if (other == this) {
				return true;
			}
			if (other == null || other.getClass() != getClass()) {
				return false;
			}
			TableState other2 = (TableState)other;
			return name.equals(other2.name) && production.equals(other2.production) && 
					dotIndex == other2.dotIndex && startCol == other2.startCol;
		}
		
		@Override
		public String toString()
		{
			String s = "";
			for (int i = 0; i < production.size(); i++) {
				if (i == dotIndex) {
					s += "\u00B7 ";
				}
				ProductionTerm t = production.get(i);
				if (t instanceof Rule) {
					s += ((Rule)t).name;
				}
				else {
					s += t;
				}
				s += " ";
			}
			if (dotIndex == production.size()) {
				s += "\u00B7";
			}
			return String.format("%-6s -> %-20s [%d-%d]", name, s, startCol.index, endCol.index);
		}
	}
	
	/*
	 * Represents a column in the Earley parsing table
	 */
	protected static class TableColumn implements Iterable<TableState>
	{
		public final String token;
		public final int index;
		public final ArrayList<TableState> states;
		
		public TableColumn(int index, String token) {
			this.index = index;
			this.token = token;
			this.states = new ArrayList<TableState>();
		}
		
		/*
		 * only insert a state if it is not already contained in the list of states. return the
		 * inserted state, or the pre-existing one. 
		 */
		public TableState insert(TableState state) {
			int index = states.indexOf(state);
			if (index < 0) {
				states.add(state);
				state.endCol = this;
				return state;
			}
			else {
				return states.get(index);
			}
		}
		
		public int size() {
			return states.size();
		}
		public TableState get(int index) {
			return states.get(index);
		}
		
		/*
		 * since we may modify the list as we traverse it, the built-in list iterator is not
		 * suitable. this iterator wouldn't mind the list being changed.
		 */
		private class ModifiableIterator implements Iterator<TableState>
		{
			private int i = 0;
			@Override
			public boolean hasNext() {
				return i < states.size();
			}
			@Override
			public TableState next() {
				TableState st = states.get(i);
				i++;
				return st;
			}
			@Override
			public void remove() {
			}
		}
		
		@Override
		public Iterator<TableState> iterator() {
			return new ModifiableIterator();
		}
		
		public void print(PrintStream out, boolean showUncompleted)
		{
			out.printf("[%d] '%s'\n", index, token);
			out.println("=======================================");
			for (TableState state : this) {
				if (!state.isCompleted() && !showUncompleted) {
					continue;
				}
				out.println(state);
			}
			out.println();
		}
	}
	
	/*
	 * A generic tree node
	 */
	public static class Node<T> implements Iterable<Node<T>>
	{
		public final T value;
		private List<Node<T>> children;
		
		public Node(T value, List<Node<T>> children) {
			this.value = value;
			this.children = children;
		}
		
		@Override
		public Iterator<Node<T>> iterator() {
			return children.iterator();
		}
		
		public void print(PrintStream out)
		{
			print(out, 0);
		}
		
		private void print(PrintStream out, int level) {
			String indentation = "";
			for (int i = 0; i < level; i++) {
				indentation += "  ";
			}
			out.println(indentation + value);
			for (Node<T> child : children) {
				child.print(out, level + 1);
			}
		}
	}
	
	/*
	 * the exception raised by Parser should parsing fail
	 */
	public static class ParsingFailed extends Exception
	{
		private static final long serialVersionUID = -3489519608069949690L;

		public ParsingFailed(String message) {
			super(message);
		}
	}
	
	/*
	 * The Earley Parser.
	 * 
	 * Usage:
	 * 
	 *     Parser p = new Parser(StartRule, "my space-delimited statement");
	 *     for (Node tree : p.getTrees()) {
	 *         tree.print(System.out);
	 *     }
	 * 
	 */
	public static class Parser
	{
		protected TableColumn[] columns;
		protected TableState finalState = null;
		
		/*
		 * constructor: takes a start rule and a statement (made of space-separated words).
		 * it initializes the table and invokes earley's algorithm
		 */
		public Parser(Rule startRule, String text) throws ParsingFailed
		{
			String[] tokens = text.split(" ");
			columns = new TableColumn[tokens.length + 1];
			columns[0] = new TableColumn(0, "");
			for (int i = 1; i <= tokens.length; i++) {
				columns[i] = new TableColumn(i, tokens[i-1]);
			}
			
			finalState = parse(startRule);
			if (finalState == null) {
				throw new ParsingFailed("your grammar does not accept the given text");
			}
		}

		// this is the name of the special "gamma" rule added by the algorithm 
		// (this is unicode for 'LATIN SMALL LETTER GAMMA')
		private static final String GAMMA_RULE = "\u0263";      // "\u0194"

		/*
		 * the Earley algorithm's core: add gamma rule, fill up table, and check if the gamma rule
		 * spans from the first column to the last one. return the final gamma state, or null,
		 * if the parse failed.
		 */
		protected TableState parse(Rule startRule)
		{
			columns[0].insert(new TableState(GAMMA_RULE, new Production(startRule), 0, columns[0]));

			for (int i = 0; i < columns.length; i++) {
				TableColumn col = columns[i];
				for (TableState state : col) {
					if (state.isCompleted()) {
						complete(col, state);
					}
					else {
						ProductionTerm term = state.getNextTerm();
						if (term instanceof Rule) {
							predict(col, (Rule)term);
						}
						else if (i + 1 < columns.length) {
							scan(columns[i+1], state, ((Terminal)term).value);
						}
					}
				}
				handleEpsilons(col);
				
				// DEBUG -- uncomment to print the table during parsing, column after column
				//col.print(System.out, false);
			}
			
			// find end state (return null if not found)
			for (TableState state : columns[columns.length - 1]) {
				if (state.name.equals(GAMMA_RULE) && state.isCompleted()) {
					return state;
				}
			}
			return null;
		}
		
		/*
		 * Earley scan
		 */
		private void scan(TableColumn col, TableState state, String token) {
		    if (token.equals(col.token)) {
			    col.insert(new TableState(state.name, state.production, state.dotIndex + 1, state.startCol));
		    }
		}
		
		/*
		 * Earley predict. returns true if the table has been changed, false otherwise
		 */
		private boolean predict(TableColumn col, Rule rule) {
			boolean changed = false;
		    for (Production prod : rule) {
		    	TableState st = new TableState(rule.name, prod, 0, col);
		    	TableState st2 = col.insert(st);
		    	changed |= (st == st2);
		    }
		    return changed;
		}
		
		/*
		 * Earley complete. returns true if the table has been changed, false otherwise
		 */
		private boolean complete(TableColumn col, TableState state) {
			boolean changed = false;
		    for (TableState st : state.startCol) {
		    	ProductionTerm term = st.getNextTerm();
		    	if (term instanceof Rule && ((Rule)term).name.equals(state.name)) {
		    		TableState st1 = new TableState(st.name, st.production, st.dotIndex + 1, st.startCol);
		            TableState st2 = col.insert(st1);
		            changed |= (st1 == st2);
		    	}
		    }
		    return changed;
		}
		
		/*
		 * call predict() and complete() for as long as the table keeps changing (may only happen 
		 * if we've got epsilon transitions)
		 */
		private void handleEpsilons(TableColumn col)
		{
			boolean changed = true;
			
			while (changed) {
				changed = false;
				for (TableState state : col) {
					ProductionTerm term = state.getNextTerm();
					if (term instanceof Rule) {
						changed |= predict(col, (Rule)term);
					}
					if (state.isCompleted()) {
						changed |= complete(col, state);
					}
				}
			}
		}
		
		/*
		 * return all parse trees (forest). the forest is simply a list of root nodes, each 
		 * representing a possible parse tree. a node is contains a value and the node's children,
		 * and supports pretty-printing
		 */
		public List<Node<TableState>> getTrees() {
			return buildTrees(finalState);
		}

		/*
		 * this is a bit "magical" -- i wrote the code that extracts a single parse tree,
		 * and with some help from a colleague (non-student) we managed to make it return all 
		 * parse trees.
		 * 
		 * how it works: suppose we're trying to match [X -> Y Z W]. we go from finish-to-start, 
		 * e.g., first we'll try to match W in X.endCol. let this matching state be M1. next we'll
		 * try to match Z in M1.startCol. let this matching state be M2. and finally, we'll try to
		 * match Y in M2.startCol, which must also start at X.startCol. let this matching state be
		 * M3.
		 * 
		 * if we matched M1, M2 and M3, then we've found a parsing for X:
		 * 
		 * X ->
		 *     Y -> M3
		 *     Z -> M2
		 *     W -> M1
		 * 
		 */
		private List<Node<TableState>> buildTrees(TableState state) {
			return buildTreesHelper(new ArrayList<Node<TableState>>(), state, 
					state.production.rules.size() - 1, state.endCol);
		}
		
		private List<Node<TableState>> buildTreesHelper(List<Node<TableState>> children,
				TableState state, int ruleIndex, TableColumn endCol)
		{
			ArrayList<Node<TableState>> outputs = new ArrayList<Node<TableState>>();
			TableColumn startCol = null;
		    if (ruleIndex < 0) {
		    	// this is the base-case for the recursion (we matched the entire rule)
		    	outputs.add(new Node<TableState>(state, children));
		    	return outputs;
		    }
		    else if (ruleIndex == 0) {
		    	// if this is the first rule
		    	startCol = state.startCol;
		    }
		    Rule rule = state.production.rules.get(ruleIndex);
		    
		    for (TableState st : endCol) {
		        if (st == state) {
		        	// this prevents an endless recursion: since the states are filled in order of
		        	// completion, we know that X cannot depend on state Y that comes after it X 
		        	// in chronological order
		            break;
		        }
		        if (!st.isCompleted() || !st.name.equals(rule.name)) {
		        	// this state is out of the question -- either not completed or does not match 
		        	// the name
		        	continue;
		        }
		        if (startCol != null && st.startCol != startCol) {
		        	// if startCol isn't null, this state must span from startCol to endCol
		            continue;
		        }
		        // okay, so `st` matches -- now we need to create a tree for every possible 
		        // sub-match
		        for (Node<TableState> subTree : buildTrees(st)) {
		        	// in python: children2 = [subTree] + children
		        	ArrayList<Node<TableState>> children2 = new ArrayList<Node<TableState>>();
		        	children2.add(subTree);
		        	children2.addAll(children);
		        	// now try all options
		            for (Node<TableState> node : buildTreesHelper(children2, state, ruleIndex - 1, st.startCol)) {
		            	outputs.add(node);
		            }
		        }
		    }
		    return outputs;
		}
	}
	
	public static void main(String[] args) throws Exception 
	{
		///////////////////////////////////////////////////////////////////////////////////////
		// Simple mathematics expressions
		///////////////////////////////////////////////////////////////////////////////////////
		Rule SYM = new Rule("SYM", new Production("a"));
		Rule OP = new Rule("OP", new Production("+"));
		Rule EXPR = new Rule("EXPR", new Production(SYM));
		EXPR.add(new Production(EXPR, OP, EXPR));

		// note that this yields the catalan numbers sequence (as expected) -- the number of ways
		// to place parenthesis in the expression a + a + ... + a
		// this serves as a "semi-proof of correctness" :)
		System.out.println("catalan numbers:");
		for (String text : new String[] {"a", "a + a", "a + a + a", "a + a + a + a", "a + a + a + a + a", 
				"a + a + a + a + a + a", "a + a + a + a + a + a + a"}) {
			Parser p = new Parser(EXPR, text);
			System.out.printf("%d, ", p.getTrees().size());
		}
		System.out.println("\n");
		
		///////////////////////////////////////////////////////////////////////////////////////
		// Simple rules for English
		///////////////////////////////////////////////////////////////////////////////////////
		Rule N = new Rule("N", new Production("time"), new Production("flight"), new Production("banana"), 
				new Production("flies"), new Production("boy"), new Production("telescope"));
		Rule D = new Rule("D", new Production("the"), new Production("a"), new Production("an"));
		Rule V = new Rule("V", new Production("book"), new Production("ate"), new Production("sleep"), 
				new Production("saw"), new Production("thinks"));
		Rule P = new Rule("P", new Production("with"), new Production("in"), new Production("on"), 
				new Production("at"), new Production("through"));
		Rule C = new Rule("C", new Production("that"));

		Rule PP = new Rule("PP");
		Rule NP = new Rule("NP", new Production(D, N), new Production("john"), new Production("bill"),
				new Production("houston"));
		NP.add(new Production(NP, PP));
		PP.add(new Production(P, NP));

		Rule VP = new Rule("VP", new Production(V, NP));
		VP.add(new Production(VP, PP));
		Rule S = new Rule("S", new Production(NP, VP), new Production(VP));
		Rule Sbar = new Rule("S'", new Production(C, S));
		VP.add(new Production(V, Sbar));
		
		// let's parse some sentences!
		for (String text : new String[] {"john ate a banana", "book the flight through houston", 
				"john saw the boy with the telescope", "john thinks that bill ate a banana"}) {
			Parser p = new Parser(S, text);
			System.out.printf("Parse trees for '%s'\n", text);
			System.out.println("===================================================");
			for (Node<TableState> tree : p.getTrees()) {
				tree.print(System.out);
				System.out.println();
			}
		}
		
		// let's fail
		boolean failed = false;
		try {
			Parser p = new Parser(S, "john ate");
		}
		catch (ParsingFailed ex) {
			// okay
			failed = true;
			System.out.println("hurrah, this has failed (we don't allow VP without a complement)");
		}
		if (!failed) {
			System.out.println("oops, this should have failed!");
		}
		
	}

}

