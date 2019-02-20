#include "syntax.hh"

#define Tk(className)                                     \
    className::className() {}                             \
    className::~className() {}                            \
    void className::Accept(SyntaxTokenVisitor& visitor) { \
        visitor.Visit(*this);                             \
    }
#include "syntax-kinds.def"
#undef Tk