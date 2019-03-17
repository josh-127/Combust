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
    BacktrackingLexer::Marker marker{ l->Mark() };

    if (Rc<PrimaryExpression> obj{ ParsePrimaryExpression(l) }; obj) {
        if (Rc<SyntaxToken> lBracket{ l->Accept<LBracketSymbol>() }; lBracket) {
            if (Rc<Expression> index{ ParseExpression(l) }; index) {
                if (Rc<SyntaxToken> rBracket{ l->Accept<RBracketSymbol>() }; rBracket) {
                    Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
                    result->SetChildren({ obj, lBracket, index, rBracket });

                    return result;
                }
            }
        }
        else if (Rc<SyntaxToken> accessor{ l->Accept<DotSymbol, MinusGtSymbol>() }; accessor) {
            if (Rc<SyntaxToken> memberName{ l->Accept<IdentifierToken>() }; memberName) {
                Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
                result->SetChildren({ obj, accessor, memberName });

                return result;
            }
        }
        else if (Rc<SyntaxToken> op{ l->Accept<PlusPlusSymbol, MinusMinusSymbol>() }; op) {
            Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
            result->SetChildren({ obj, op });

            return result;
        }

        return obj;
    }

    l->Backtrack(marker);
    return Rc<Expression>{ };
}

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer) {
    return ParsePostfixExpression(lexer);
}