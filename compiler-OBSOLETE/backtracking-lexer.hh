#ifndef COMBUST_BACKTRACKING_LEXER_HH
#define COMBUST_BACKTRACKING_LEXER_HH
#include "common.hh"
#include "lexer.hh"
#include "syntax.hh"

struct BACKTRACKING_LEXER_IMPL;

class BacktrackingLexer : public Object, public virtual ILexer {
public:
    using Marker = size_t;

    explicit BacktrackingLexer(Rc<ILexer> lexer);
    virtual ~BacktrackingLexer();

    Rc<SyntaxToken> ReadToken() override;
    Rc<SyntaxToken> PeekToken();
    Marker Mark();
    void Backtrack(const Marker& to);

    template<typename T>
    [[nodiscard]] Rc<SyntaxToken> AcceptSingle() {
        Rc<SyntaxToken> token{ PeekToken() };
        if (IsSyntaxNode<T>(token)) {
            ReadToken();
            return token;
        }
        return Rc<SyntaxToken>{ };
    }

    template<typename First, typename... Types>
    [[nodiscard]] Rc<SyntaxToken> Accept() {
        if constexpr (sizeof...(Types) > 0) {
            if (Rc<SyntaxToken> token{ AcceptSingle<First>() }; token)
                return token;
            return Accept<Types...>();
        }
        else {
            return AcceptSingle<First>();
        }
    }

    template<typename T>
    [[nodiscard]] Rc<SyntaxToken> Expect() {
        if (Rc<SyntaxToken> token{ AcceptSingle<T>() }; token)
            return token;
        Rc<T> missingToken{ NewObj<T>() };
        missingToken->SetFlag(SyntaxToken::IS_MISSING);
        return missingToken;
    }

private:
    Owner<BACKTRACKING_LEXER_IMPL> l;
};

#endif
