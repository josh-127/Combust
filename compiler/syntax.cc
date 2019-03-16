#include "syntax.hh"

#define O(className)          \
    className::className() {} \
    className::~className() {}
#define Sn(className)
#define Tk(className) O(className)
#include "syntax-kinds.def"
#undef Tk
#undef Sn
#undef O

#define O(className)                                          \
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

bool PrimaryExpression::IsIdentifier() const {
    return children.size() == 1 && IsSyntaxNode<IdentifierToken>(children[0]);
}
bool PrimaryExpression::IsNumericLiteral() const {
    return children.size() == 1 && IsSyntaxNode<NumericLiteralToken>(children[0]);
}
bool PrimaryExpression::IsStringLiteral() const {
    return children.size() == 1 && IsSyntaxNode<StringLiteralToken>(children[0]);
}
bool PrimaryExpression::IsParenthesizedExpression() const {
    return children.size() == 3
        && IsSyntaxNode<LParenSymbol>(children[0])
        && IsBaseOfSyntaxNode<Expression>(children[1])
        && IsSyntaxNode<RParenSymbol>(children[2]);
}
bool PrimaryExpression::IsValid() const {
    return IsIdentifier()
        || IsNumericLiteral()
        || IsStringLiteral()
        || IsParenthesizedExpression();
}
