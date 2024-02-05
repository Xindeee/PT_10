grammar MyGrammar;

@header {
#include <string>
#include <variant>
}

@parser::members {
enum TOK_TYPE {
  TOK_TYPE_I,
  TOK_TYPE_S,
  TOK_TYPE_INC
};

struct Tok_t {
  TOK_TYPE e;
  std::variant<int, std::string, std::pair<std::string, bool>> v;
};
}

expr returns [ int i ]: term EQUAL expr EOF | add;
add returns [ int i ]: add PLUS term | term;

term returns [ Tok_t tok ]
    : PLUSPLUS s=STR { $tok.e = TOK_TYPE_INC; $tok.v = std::make_pair($s.text, false); }
    | s=STR { $tok.e = TOK_TYPE_S; $tok.v = $s.text; }
    | c=comp { $tok = $c.tok; };

comp returns [ Tok_t tok ]
    : s=STR PLUSPLUS { $tok.e = TOK_TYPE_INC; $tok.v = std::make_pair($s.text, true); }
    | i=INT { $tok.e = TOK_TYPE_I; $tok.v = $i.int; };

WHITESPACE: [ \r\t\n]+ -> skip;
STR: [a-zA-Z][a-zA-Z0-9]*;
PLUS: '+';
PLUSPLUS: '++';
EQUAL: '=';
INT: [0-9][0-9]*;
