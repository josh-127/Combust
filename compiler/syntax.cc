#include "syntax.hh"

#define O(className)                                          \
    className::className() {}                                  \
    className::~className() {}                                 \
    Rc<Object> className::Accept(SyntaxNodeVisitor& visitor) { \
        return visitor.Visit(*this);                           \
    }                                                          \
    class className
#define Sn(className) O(className)
#define Tk(className) O(className)
#include "syntax-kinds.def"
#undef Tk
#undef Sn
#undef O
