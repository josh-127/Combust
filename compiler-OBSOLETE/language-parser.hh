#ifndef COMBUST_LANGUAGE_PARSER_HH
#define COMBUST_LANGUAGE_PARSER_HH
#include "common.hh"

class BacktrackingLexer;

class Expression;
class Declaration;
class Statement;
class SyntaxNode;

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer);

#endif
