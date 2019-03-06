#include "syntax.hh"

#define Tk(className)                                          \
    className::className() {}                                  \
    className::~className() {}                                 \
    Rc<Object> className::Accept(SyntaxNodeVisitor& visitor) { \
        return visitor.Visit(*this);                           \
    }                                                          \
    class className
#include "syntax-kinds.def"
#undef Tk
