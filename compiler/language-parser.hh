#ifndef COMBUST_LANGUAGE_PARSER_HH
#define COMBUST_LANGUAGE_PARSER_HH
#include "common.hh"

class SyntaxNode;
class Expression;
class Statement;
class Declaration;

Rc<Expression> ParseExpression();

#endif
