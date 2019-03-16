#include "language-parser.hh"
#include "backtracking-lexer.hh"
#include "syntax.hh"

static Rc<PrimaryExpression> ParsePrimaryExpression(Rc<BacktrackingLexer> l) {
    BacktrackingLexer::Marker marker{ l->Mark() };

    Rc<SyntaxToken> token{ l->ReadToken() };

    if (IsSyntaxNode<IdentifierToken>(token) ||
        IsSyntaxNode<NumericLiteralToken>(token) ||
        IsSyntaxNode<StringLiteralToken>(token))
    {
        Rc<PrimaryExpression> expression{ NewObj<PrimaryExpression>() };
        expression->SetChildren({ token });

        return expression;
    }
#if 0
    else if (IsSyntaxNode<LParenSymbol>(token)) {
    }
#endif

    l->Backtrack(marker);
    return Rc<PrimaryExpression>{ };
}

static Rc<Expression> ParsePostfixExpression(Rc<BacktrackingLexer> l) {
    BacktrackingLexer::Marker marker{ l->Mark() };

    Rc<PrimaryExpression> expression{ ParsePrimaryExpression(l) };

    if (expression) {
        marker = l->Mark();

        SyntaxNodeVector children{ };
        children.push_back(expression);

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

        if (!hasPostfix) {
            l->Backtrack(marker);
        }

        Rc<PostfixExpression> result{ NewObj<PostfixExpression>() };
        result->SetChildren(children);
        return result;
    }

    l->Backtrack(marker);
    return Rc<Expression>{ };
}

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer) {
    return ParsePostfixExpression(lexer);
}