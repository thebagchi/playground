from ply.lex import lex as lex
from ply.yacc import yacc as yacc


class LexerError(Exception):
    pass


class ParserError(Exception):
    pass


tokens = [
    "CURLY_START",
    "CURLY_END",
    "SQUARE_START",
    "SQUARE_END",
    "COMMA",
    "COLON",
    "NUMBER",
    "STRING",
    "TRUE",
    "FALSE",
    "NULL",
]

t_ignore = " \t\n"


def t_CURLY_START(t):
    r"\{"
    return t


def t_CURLY_END(t):
    r"\}"
    return t


def t_SQUARE_START(t):
    r"\["
    return t


def t_SQUARE_END(t):
    r"\]"
    return t


def t_COMMA(t):
    r","
    return t


def t_COLON(t):
    r":"
    return t


def t_TRUE(t):
    r"true"
    t.value = True
    return t


def t_FALSE(t):
    r"false"
    t.value = False
    return t


def t_NULL(t):
    r"null"
    t.value = None
    return t


def t_STRING(t):
    r'"(([^"\\])|(\\["\\\/bfnrt])|(\\u[0-9a-f]{4}))*"'
    t.value = t.value[1:-1]  # Remove quotation marks
    return t


def t_NUMBER(t):
    r"\-?(0|([1-9][0-9]*))(\.[0-9]*)?([eE][\+\-]?[0-9]*)?"
    try:
        t.value = int(t.value)
    except:
        t.value = float(t.value)
    return t


def t_error(t):
    raise LexerError(t)


def p_Value(p):
    """Value : STRING
    | NUMBER
    | CURLY_START Object CURLY_END
    | SQUARE_START List SQUARE_END
    | Bool
    | NULL
    """
    if len(p) == 4:
        p[0] = p[2]
    else:
        p[0] = p[1]


def p_Bool(p):
    """
    Bool : TRUE
           | FALSE
    """
    p[0] = p[1]


def p_List(p):
    """
    List : List COMMA Value
           | Value
    """
    if len(p) == 2:
        # Value
        p[0] = [p[1]]
    elif len(p) == 4:
        # List COMMA Value
        p[0] = p[1] + [p[3]]


def p_List_empty(p):
    """List : empty"""
    p[0] = []


def p_Object(p):
    """
    Object : Object COMMA STRING COLON Value
             | STRING COLON Value
    """
    if len(p) == 4:
        # STRING COLON Value
        p[0] = {p[1]: p[3]}
    elif len(p) == 6:
        # Object COMMA STRING COLON Value
        p[0] = p[1].copy()
        p[0][p[3]] = p[5]


def p_Object_empty(p):
    """Object : empty"""
    p[0] = {}


def p_empty(p):
    """empty :"""


def p_error(p):
    print(p)
    raise ParserError(p)


lexer = lex()
parser = yacc()


def main(data):

    lexer.input(data)
    result = parser.parse(data)
    print(result)
    pass


if __name__ == "__main__":
    data = """
{ 
    "hello" : 12345,
    "world" : "hello",
    "list" : [1, 2],
    "null" : null,
    "bool" : true
}
"""
    main(data)
