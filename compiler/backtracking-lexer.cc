#include "backtracking-lexer.hh"
#include "lexer.hh"
#include "syntax.hh"
#include <vector>

struct BACKTRACKING_LEXER_IMPL {
    std::vector<Rc<SyntaxToken>> tokens{ };
    BacktrackingLexer::Marker currentPos{ 0 };
};

BacktrackingLexer::BacktrackingLexer(Rc<ILexer> lexer) :
    l{ NewChild<BACKTRACKING_LEXER_IMPL>() }
{
    Rc<SyntaxToken> token{ };
    do {
        token = lexer->ReadToken();
        l->tokens.push_back(token);
    }
    while (!IsSyntaxNode<EofToken>(token));

    l->tokens.push_back(token);
}

BacktrackingLexer::~BacktrackingLexer() {}

Rc<SyntaxToken> BacktrackingLexer::ReadToken() {
    Rc<SyntaxToken> token{ l->tokens[l->currentPos] };

    if (l->currentPos + 1 < l->tokens.size())
        ++l->currentPos;

    return token;
}

BacktrackingLexer::Marker BacktrackingLexer::Mark() {
    return l->currentPos;
}

void BacktrackingLexer::Backtrack(const Marker& to) {
    l->currentPos = to;
}