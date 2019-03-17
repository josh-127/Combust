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

    Rc<PrimaryExpression> primaryExpression{ ParsePrimaryExpression(l) };

    if (primaryExpression) {
        marker = l->Mark();

        SyntaxNodeVector children{ };
        children.push_back(primaryExpression);

        Rc<SyntaxToken> firstToken{ l->ReadToken() };
        bool hasPostfix{ false };

        if (IsSyntaxNode<LBracketSymbol>(firstToken)) {
            Rc<Expression> expression{ ParseExpression(l) };
            if (expression) {
                Rc<SyntaxToken> secondToken{ l->ReadToken() };
                if (IsSyntaxNode<RBracketSymbol>(secondToken)) {
                    children.push_back(firstToken);
                    children.push_back(expression);
                    children.push_back(secondToken);
                    hasPostfix = true;
                }
            }
        }
        else if (IsSyntaxNode<DotSymbol>(firstToken)
            || IsSyntaxNode<MinusGtSymbol>(firstToken))
        {
            Rc<SyntaxToken> identifier{ l->ReadToken() };
            if (IsSyntaxNode<IdentifierToken>(identifier)) {
                children.push_back(firstToken);
                children.push_back(identifier);
                hasPostfix = true;
            }
        }
        else if (IsSyntaxNode<PlusPlusSymbol>(firstToken)
            || IsSyntaxNode<MinusMinusSymbol>(firstToken))
        {
            children.push_back(firstToken);
            hasPostfix = true;
        }

        if (hasPostfix) {
            Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
            result->SetChildren(children);
            return result;
        }

        l->Backtrack(marker);
        return primaryExpression;
    }

    l->Backtrack(marker);
    return Rc<Expression>{ };
}

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer) {
    return ParsePostfixExpression(lexer);
}