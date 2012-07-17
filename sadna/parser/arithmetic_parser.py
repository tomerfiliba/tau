"""Module containing LEX/YACC parsing code for the Arithmetic-variety language.
"""

from ply import lex, yacc
import re

tokens = (
    'NUMBER',
    'NAME',
    'PLUS',
    'MINUS',
    'TIMES',
    'EQUALS',
    'LPAREN',
    'RPAREN',
    'LCURLY',
    'RCURLY',
    'COMMA',
    'NEWLINE',
)

t_NAME    = r'[a-zA-Z_][a-zA-Z0-9_]*'
t_PLUS    = r'\+'
t_MINUS   = r'-'
t_TIMES   = r'\*'
t_EQUALS  = r'='
t_LPAREN  = r'\('
t_RPAREN  = r'\)'
t_LCURLY  = r'\{'
t_RCURLY  = r'\}'
t_COMMA   = r','
t_ignore  = ' \t'


def t_NUMBER(t):
  r'[0-9]*\.?[0-9]+'
  t.value = float(t.value)
  return t

def t_NEWLINE(t):
  r'\n+'
  t.lexer.lineno += t.value.count("\n")
  return t

def t_error(t):
  print "Illegal character '%s'" % t.value[0]
  t.lexer.skip(1)

lexer = lex.lex()


precedence = (
    ('left', 'COMMA'),
    ('left', 'NEWLINE'),
    ('nonassoc', 'EQUALS'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES'),
    ('left', 'MULT')
)


def p_functions(p):
  '''functions : functions functions'''
  p[0] = p[1] + p[2]

def p_function(p):
  '''functions : NAME LPAREN paramlist RPAREN lcurly equations rcurly'''
  p[0] = [{
      'fname': p[1],
      'params': p[3],
      'equations': p[6],
  }]

def p_no_newline(p):
  '''lcurly : LCURLY
            | lcurly NEWLINE
            | NEWLINE lcurly
     rcurly : RCURLY
            | RCURLY NEWLINE
     comma  : COMMA
            | COMMA NEWLINE
            | NEWLINE comma'''
  pass

def p_paramlist_comma(p):
  '''paramlist : paramlist comma paramlist'''
  p[0] = p[1] + p[3]

def p_paramlist_from_name(p):
  '''paramlist : NAME'''
  p[0] = [p[1]]

def p_paramlist_empty(p):
  '''paramlist :'''
  p[0] = []

def p_paramlist_newline(p):
  '''paramlist : paramlist NEWLINE'''
  p[0] = p[1]

def p_equations_newline_equations(p):
  '''equations : equations NEWLINE equation'''
  p[0] = p[1] + p[3]

def p_equations_newline(p):
  '''equations : equations NEWLINE'''
  p[0] = p[1]

def p_equations_empty(p):
  '''equations :'''
  p[0] = []

def p_equations_one(p):
  '''equations : equation'''
  p[0] = [p[1]]

def p_equation(p):
  '''equation : term EQUALS term'''
  d = dict(p[1][0])
  for var, quantifier in p[3][0].iteritems():
    if var not in d:
      d[var] = 0
    d[var] -= quantifier
  p[0] = [(d, p[3][1] - p[1][1])]



def p_term_plusminus_term(p):
  '''term : term PLUS term
          | term MINUS term'''
  minusmult = 1
  if re.match(t_MINUS, p[2]):
    minusmult = -1
  d = dict(p[1][0])
  for var, quantifier in p[3][0].iteritems():
    if var not in d:
      d[var] = 0
    d[var] += minusmult * quantifier
  p[0] = (d, p[1][1] + minusmult*p[3][1])

def p_term_times_literal(p):
  '''term : term TIMES literal'''
  d = dict(p[1][0])
  for var in d:
    d[var] *= p[3]
  p[0] = (d, p[1][1] * p[3])

def p_literal_times_term(p):
  '''term : literal TIMES term   %prec MULT'''
  d = dict(p[3][0])
  for var in d:
    d[var] *= p[1]
  p[0] = (d, p[3][1] * p[1])

def p_term_parenthesis(p):
  '''term : LPAREN term RPAREN'''
  p[0] = p[2]

def p_literal_number(p):
  '''literal : NUMBER'''
  p[0] = p[1]

def p_literal_minus_numer(p):
  '''literal : MINUS NUMBER'''
  p[0] = -p[2]

def p_term_literal(p):
  '''term : literal'''
  p[0] = ({}, p[1])

def p_term_name(p):
  '''term : NAME'''
  p[0] = ({p[1]: 1}, 0)

def p_term_minus_name(p):
  '''term : MINUS NAME'''
  p[0] = ({p[2]: -1}, 0)

def p_error(p):
  raise SyntaxError, "Syntax error at %s on line %d" % (repr(p.value), p.lineno)

parser = yacc.yacc(tabmodule="autogen_arithmetic_parsetab", debug=0)
expression_parser = yacc.yacc(tabmodule="autogen_arithexpr_parsetab", start="term", debug=0)


def parse(code):
  """Parses the input string as Arithmetic variety language.

  @param code: Code which is valid under the Arithmetic variety specifications.
  @return
    List of function specifications, with each function in the format:
    Dictionary with the following keys: 'fname' for function name, 'params' for
    a list of parameters for this function and 'equations' for a list of
    equations in the code. Each equation is a tuple of the form:
    (dict of parameters with coefficients for values, equations solution)
  """
  return parser.parse(code.strip(), lexer=lexer)

def parse_expression(code):
    """
    Parses the input string as an expression intended for embedding in a
    higher-level construct.
    
    @param code: a string constituting a valid expression of the arithmetic
      variety.
    @return a linear expression as a tuple of two items:
      - a dictionary of coefficients of the form {'v': coeff, ...}
      - the free term
      E.g., a-b*3+9 -> ({'a': 1, 'b': -3.0}, 9.0)
    """
    return expression_parser.parse(code.strip(), lexer=lexer)



if __name__ == '__main__':
    print parse_expression("a+5+9")
    print parse("f() { x = a+b * 3 } g(x,y) { x*2=y\n y*2=z }")
    print parse_expression("5+7*a")