#include "language-parser.hh"
#include "backtracking-lexer.hh"
#include "syntax.hh"

static Rc<PrimaryExpression> ParsePrimaryExpression(Rc<BacktrackingLexer> l) {
    BacktrackingLexer::Marker marker{ l->Mark() };

    if (Rc<SyntaxToken> token{ l->Accept<IdentifierToken,
                                         NumericLiteralToken,
                                         StringLiteralToken>() }; token)
    {
        Rc<PrimaryExpression> expression{ NewObj<PrimaryExpression>() };
        expression->SetChildren({ token });

        return expression;
    }
    else if (Rc<SyntaxToken> lParen{ l->Accept<LParenSymbol>() }; lParen) {
        if (Rc<Expression> innerExpression{ ParseExpression(l) }; innerExpression) {
            if (Rc<SyntaxToken> rParen{ l->Accept<RParenSymbol>() }; rParen) {
                Rc<PrimaryExpression> expression{ NewObj<PrimaryExpression>() };
                expression->SetChildren({ lParen, innerExpression, rParen });

                return expression;
            }
        }
    }

    l->Backtrack(marker);
    return Rc<PrimaryExpression>{ };
}

static Rc<Expression> ParsePostfixExpression(Rc<BacktrackingLexer> l) {
    Rc<Expression> obj{ ParsePrimaryExpression(l) };
    BacktrackingLexer::Marker marker{ l->Mark() };
    bool isDone{ false };

    while (!isDone) {
        Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
        isDone = true;

        if (Rc<SyntaxToken> lBracket{ l->Accept<LBracketSymbol>() }; lBracket) {
            if (Rc<Expression> index{ ParseExpression(l) }; index) {
                if (Rc<SyntaxToken> rBracket{ l->Accept<RBracketSymbol>() }; rBracket) {
                    result->SetChildren({ obj, lBracket, index, rBracket });
                    isDone = false;
                }
            }
        }
        else if (Rc<SyntaxToken> accessor{ l->Accept<DotSymbol, MinusGtSymbol>() }; accessor) {
            if (Rc<SyntaxToken> memberName{ l->Accept<IdentifierToken>() }; memberName) {
                result->SetChildren({ obj, accessor, memberName });
                isDone = false;
            }
        }
        else if (Rc<SyntaxToken> op{ l->Accept<PlusPlusSymbol, MinusMinusSymbol>() }; op) {
            result->SetChildren({ obj, op });
            isDone = false;
        }

        if (!isDone) {
            obj = result;
        }
    }

    return obj;
}

static Rc<Expression> ParseUnaryExpression(Rc<BacktrackingLexer> l) {
    BacktrackingLexer::Marker marker{ l->Mark() };

    if (Rc<SyntaxToken> op{ l->Accept<PlusPlusSymbol, MinusMinusSymbol>() }; op) {
    }
    else if (Rc<SyntaxToken> op{ l->Accept<AmpersandSymbol,
                                           AsteriskSymbol,
                                           PlusSymbol,
                                           MinusSymbol,
                                           TildeSymbol,
                                           ExclamationSymbol>() }; op)
    {
    }
    else if (Rc<SyntaxToken> op{ l->Accept<SizeOfKeyword>() }; op) {
    }
    else if (Rc<Expression> expression{ ParsePostfixExpression(l) }; expression) {
        return expression;
    }

    l->Backtrack(marker);
    return Rc<Expression>{ };
}

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer) {
    return ParseUnaryExpression(lexer);
}