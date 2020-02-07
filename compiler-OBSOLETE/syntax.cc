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

bool PostfixExpression::IsPassthrough() const {
    return children.size() == 1 && IsSyntaxNode<PrimaryExpression>(children[0]);
}
bool PostfixExpression::IsArrayAccessor() const {
    return children.size() == 4
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<LBracketSymbol>(children[1])
        && IsBaseOfSyntaxNode<Expression>(children[2])
        && IsSyntaxNode<RBracketSymbol>(children[3]);
}
bool PostfixExpression::IsFunctionCall() const {
    return false;
}
bool PostfixExpression::IsStructureReference() const {
    return children.size() == 3
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<DotSymbol>(children[1])
        && IsSyntaxNode<IdentifierToken>(children[2]);
}
bool PostfixExpression::IsStructureDereference() const {
    return children.size() == 3
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<MinusGtSymbol>(children[1])
        && IsSyntaxNode<IdentifierToken>(children[2]);
}
bool PostfixExpression::IsPostIncrement() const {
    return children.size() == 2
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<PlusPlusSymbol>(children[1]);
}
bool PostfixExpression::IsPostDecrement() const {
    return children.size() == 2
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0]))
        && IsSyntaxNode<MinusMinusSymbol>(children[1]);
}
bool PostfixExpression::IsValid() const {
    return IsPassthrough()
        || IsArrayAccessor()
        || IsFunctionCall()
        || IsStructureReference()
        || IsStructureDereference()
        || IsPostIncrement()
        || IsPostDecrement();
}

bool UnaryExpression::IsPassthrough() const {
    return children.size() == 1
        && (IsSyntaxNode<PrimaryExpression>(children[0])
            || IsSyntaxNode<PostfixExpression>(children[0])
            || IsSyntaxNode<UnaryExpression>(children[0]));
}
bool UnaryExpression::IsPreIncrement() const {
    return children.size() == 2
        && IsSyntaxNode<PlusPlusSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsPreDecrement() const {
    return children.size() == 2
        && IsSyntaxNode<MinusMinusSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsAddressOf() const {
    return children.size() == 2
        && IsSyntaxNode<AmpersandSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsPointerDereference() const {
    return children.size() == 2
        && IsSyntaxNode<AsteriskSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsPositive() const {
    return children.size() == 2
        && IsSyntaxNode<PlusPlusSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsNegative() const {
    return children.size() == 2
        && IsSyntaxNode<MinusMinusSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsBitwiseComplement() const {
    return children.size() == 2
        && IsSyntaxNode<TildeSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsLogicalNot() const {
    return children.size() == 2
        && IsSyntaxNode<ExclamationSymbol>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsSizeOf() const {
    return children.size() == 2
        && IsSyntaxNode<SizeOfKeyword>(children[0])
        && (IsSyntaxNode<PrimaryExpression>(children[1])
            || IsSyntaxNode<PostfixExpression>(children[1])
            || IsSyntaxNode<UnaryExpression>(children[1]));
}
bool UnaryExpression::IsParenthesizedSizeOf() const {
    return children.size() == 4
        && IsSyntaxNode<SizeOfKeyword>(children[0])
        && IsSyntaxNode<LParenSymbol>(children[1])
        && (IsSyntaxNode<PrimaryExpression>(children[2])
            || IsSyntaxNode<PostfixExpression>(children[2])
            || IsSyntaxNode<UnaryExpression>(children[2]))
        && IsSyntaxNode<RParenSymbol>(children[3]);
}
bool UnaryExpression::IsValid() const {
    return IsPassthrough()
        || IsPreIncrement()
        || IsPreDecrement()
        || IsAddressOf()
        || IsPointerDereference()
        || IsPositive()
        || IsNegative()
        || IsBitwiseComplement()
        || IsLogicalNot()
        || IsSizeOf()
        || IsParenthesizedSizeOf();
}
