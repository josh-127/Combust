#ifndef COMBUST_BACKTRACKING_LEXER_HH
#define COMBUST_BACKTRACKING_LEXER_HH
#include "common.hh"
#include "lexer.hh"

struct BACKTRACKING_LEXER_IMPL;

class BacktrackingLexer : public Object, public virtual ILexer {
public:
    using Marker = int;

    explicit BacktrackingLexer(Rc<ILexer> lexer);
    virtual ~BacktrackingLexer();

    Rc<SyntaxToken> ReadToken() override;
    Marker Mark();
    void Backtrack(const Marker& to);

private:
    Owner<BACKTRACKING_LEXER_IMPL> l;
};

#endif
