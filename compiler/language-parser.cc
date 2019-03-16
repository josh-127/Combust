#include "language-parser.hh"
#include "backtracking-lexer.hh"
#include "syntax.hh"

static Rc<Expression> ParsePrimaryExpression(Rc<BacktrackingLexer> l) {
    BacktrackingLexer::Marker marker{ l->Mark() };

    Rc<SyntaxToken> token{ l->ReadToken() };

    if (IsSyntaxNode<IdentifierToken>(token) ||
        IsSyntaxNode<NumericLiteralToken>(token) ||
        IsSyntaxNode<StringLiteralToken>(token))
    {
        Rc<PrimaryExpression> expression{ NewObj<PrimaryExpression>() };
        expression->SetValue(token);

        return expression;
    }
#if 0
    else if (IsSyntaxNode<LParenSymbol>(token)) {
    }
#endif

    l->Backtrack(marker);
    return Rc<Expression>{ };
}

Rc<Expression> ParseExpression(Rc<BacktrackingLexer> lexer) {
    return ParsePrimaryExpression(lexer);
}