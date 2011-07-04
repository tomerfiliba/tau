import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
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
	public interface ProductionTerm
	{
	}
	
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

		@Override
		public int hashCode() 
		{
			return value.hashCode();
		}
	}
	
	/*
	 * represents a production of the rule. 
	 */
	public static class Production implements Iterable<ProductionTerm>
	{
		public final List<ProductionTerm> terms;
		
		public Production(ProductionTerm... terms)
		{
			this.terms = Arrays.asList(terms);
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
		
		public List<Rule> getRules()
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
		public int hashCode() 
		{
			return terms.hashCode();
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

	public static final Production Epsilon = new Production();
	
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
		public int hashCode() 
		{
			return name.hashCode();
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
	
	protected static class TableState
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
		public int hashCode()
		{
			return name.hashCode() * 31 + production.hashCode();
		}
		
		@Override
		public String toString()
		{
			String s = "";
			for (int i = 0; i < production.size(); i++) {
				if (i == dotIndex) {
					s += "$ ";
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
				s += "$";
			}
			return String.format("%-6s -> %-20s [%d-%d]", name, s, startCol.index, endCol.index);
		}
	}
	
	protected static class TableColumn implements Iterable<TableState>
	{
		public final String token;
		public final int index;
		public final ArrayList<TableState> states;
		public final HashSet<TableState> existingStates;
		
		public TableColumn(int index, String token) {
			this.index = index;
			this.token = token;
			this.states = new ArrayList<TableState>();
			this.existingStates = new HashSet<TableState>();
		}
		
		public boolean add(TableState state) {
			if (existingStates.contains(state)) {
				return false;
			}
			existingStates.add(state);
			states.add(state);
			state.endCol = this;
			return true;
		}
		
		public int size() {
			return states.size();
		}
		public TableState get(int index) {
			return states.get(index);
		}
		
		protected class ModifiableIterator implements Iterator<TableState>
		{
			protected int i = 0;
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
		
		@Override
		public String toString()
		{
			String s = "[" + index + "] '" + token + "'\n=======================================\n";
			for (TableState state : this) {
				s += state + "\n";
			}
			return s;
		}
	}
	
	public static class Node implements Iterable<Node>
	{
		public final Object value;
		public Node parent;
		protected ArrayList<Node> children;
		
		public Node(Object value) {
			this.value = value;
			children = new ArrayList<Node>();
		}
		
		public Node add(Node child) {
			children.add(child);
			child.parent = this;
			return child;
		}
		public Node add(Object value) {
			return add(new Node(value));
		}
		
		public Node getRoot() {
			Node n = this;
			while (n.parent != null) {
				n = n.parent;
			}
			return n;
		}
		
		public int size() {
			return children.size();
		}
		public int getWidth() {
			if (hasChildren()) {
				int width = 0;
				for (Node child : children) {
					width += child.getWidth();
				}
				return width;
			}
			else {
				return 1;
			}
		}
		public boolean hasChildren() {
			return children.size() > 0;
		}
		@Override
		public Iterator<Node> iterator() {
			return children.iterator();
		}
		
		protected Node duplicate() {
			Node n = new Node(value);
			for (Node child : children) {
				n.add(child.duplicate());
			}
			return n;
		}

		public void print()
		{
			print(System.out, 0);
		}
		
		protected void print(PrintStream out, int level) {
			String indentation = "";
			for (int i = 0; i < level; i++) {
				indentation += "  ";
			}
			out.println(indentation + value);
			for (Node child : children) {
				child.print(out, level + 1);
			}
		}
	}
	
	public static class ParsingFailed extends Exception
	{
		private static final long serialVersionUID = -3489519608069949690L;

		public ParsingFailed(String message) {
			super(message);
		}
	}
	
	public static class Parser
	{
		protected TableColumn[] columns;
		protected TableState finalState = null;
		
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

		private static final String SPECIAL_RULE = "&00&";

		protected TableState parse(Rule startRule)
		{
			columns[0].add(new TableState(SPECIAL_RULE, new Production(startRule), 0, columns[0]));

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
				System.out.println(col);
			}
			
			// find end state (return null if not found)
			for (TableState state : columns[columns.length - 1]) {
				if (state.name.equals(SPECIAL_RULE) && state.isCompleted()) {
					return state;
				}
			}
			return null;
		}
		
		protected void scan(TableColumn col, TableState state, String token) {
		    if (token.equals(col.token)) {
			    col.add(new TableState(state.name, state.production, state.dotIndex + 1, state.startCol));
		    }
		}
		
		protected void predict(TableColumn col, Rule rule) {
		    for (Production prod : rule) {
		    	col.add(new TableState(rule.name, prod, 0, col));
		    }
		}
		
		protected void complete(TableColumn col, TableState state) {
		    for (TableState st : state.startCol) {
		    	ProductionTerm term = st.getNextTerm();
		    	if (term instanceof Rule && ((Rule)term).name.equals(state.name)) {
		            col.add(new TableState(st.name, st.production, st.dotIndex + 1, st.startCol));
		    	}
		    }
		}
		
		public List<Node> getTrees() {
			ArrayList<Node> forest = new ArrayList<Node>();
			forest.add(buildTree3(finalState));
			return forest;
		}

		protected Node buildTree1(TableState state) {
			Node node = new Node(state);
			TableColumn endCol = state.endCol;
			List<Rule> rules = state.production.getRules();
			
			for (int i = rules.size() - 1; i >= 0; i--) {
				Rule r = rules.get(i);
				TableColumn startCol = (i == 0) ? state.startCol : null;
				for (TableState st : endCol) {
					if (st == state) {
						break;
					}
					if (!st.isCompleted() || (startCol != null && st.startCol != startCol)) {
						continue;
					}
					if (r.name.equals(st.name)) {
						node.add(buildTree1(st));
						endCol = st.startCol;
						break;
					}
				}
			}
			
			return node;
		}
		
		protected List<TableState> findMatches(TableState state, Rule rule, TableColumn startCol, TableColumn endCol)
		{
			ArrayList<TableState> matches = new ArrayList<TableState>();
			
			for (TableState st : endCol) {
				if (st == state) {
					break;
				}
				if (!st.isCompleted() || (startCol != null && st.startCol != startCol)) {
					continue;
				}
				if (rule.name.equals(st.name)) {
					matches.add(st);
				}
			}
			return matches;
		}
		
		protected Node buildTree2(TableState state) {
			Node node = new Node(state);
			TableColumn endCol = state.endCol;
			List<Rule> rules = state.production.getRules();
			
			for (int i = rules.size() - 1; i >= 0; i--) {
				TableColumn startCol = (i == 0) ? state.startCol : null;
				List<TableState> matches = findMatches(state, rules.get(i), startCol, endCol);
				if (!matches.isEmpty()) {
					TableState st = matches.get(0);
					node.add(buildTree2(st));
					endCol = st.startCol;
				}
			}
			
			return node;
		}
		
		protected Node buildTree3(TableState state) {
			Node node = new Node(state);
			TableColumn endCol = state.endCol;
			List<Rule> rules = state.production.getRules();
			
			for (int i = rules.size() - 1; i >= 0; i--) {
				TableColumn startCol = (i == 0) ? state.startCol : null;
				List<TableState> matches = findMatches(state, rules.get(i), startCol, endCol);
				if (!matches.isEmpty()) {
					TableState st = matches.get(0);
					node.add(buildTree3(st));
					endCol = st.startCol;
				}
			}
			
			return node;
		}
		

	}

	public static void main(String[] args) throws Exception {
		Rule SYM = new Rule("SYM", new Production("a"));
		Rule OP = new Rule("OP", new Production("+"), new Production("-"));
		Rule EXPR = new Rule("EXPR", new Production(SYM));
		EXPR.add(new Production(EXPR, OP, EXPR));

		Parser p = new Parser(EXPR, "a + a + a");
		ArrayList<Node> forest = (ArrayList<Node>) p.getTrees();
		for (Node n : forest) {
			n.children.get(0).print();
			System.out.println("- - - - - - - - - - - - - - - - - - -");
		}
	}

}

