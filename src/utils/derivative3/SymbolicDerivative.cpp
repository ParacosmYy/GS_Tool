/**
 * @file SymbolicDerivative.cpp
 * @brief 符号微分实现
 */

#include "utils/derivative3/SymbolicDerivative.h"

#include <QElapsedTimer>
#include <QtMath>
#include <stack>

SymbolicDerivative::SymbolicDerivative(QObject* parent)
    : QObject(parent)
{
}

/* ── 解析器 ── */

void SymbolicDerivative::skipSpaces(const QString& expr, int& pos) const
{
    while (pos < expr.size() && expr[pos].isSpace()) ++pos;
}

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::parse(
    const QString& expr) const
{
    int pos = 0;
    auto result = parseExpr(expr, pos);
    return result;
}

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::parseExpr(
    const QString& expr, int& pos) const
{
    auto left = parseTerm(expr, pos);
    skipSpaces(expr, pos);

    while (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-')) {
        QChar op = expr[pos];
        ++pos;
        auto right = parseTerm(expr, pos);
        auto node = std::make_shared<Expr>();
        node->type = (op == '+') ? ExprType::Add : ExprType::Subtract;
        node->left = left;
        node->right = right;
        left = node;
        skipSpaces(expr, pos);
    }
    return left;
}

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::parseTerm(
    const QString& expr, int& pos) const
{
    auto left = parseFactor(expr, pos);
    skipSpaces(expr, pos);

    while (pos < expr.size() && (expr[pos] == '*' || expr[pos] == '/')) {
        QChar op = expr[pos];
        ++pos;
        auto right = parseFactor(expr, pos);
        auto node = std::make_shared<Expr>();
        node->type = (op == '*') ? ExprType::Multiply : ExprType::Divide;
        node->left = left;
        node->right = right;
        left = node;
        skipSpaces(expr, pos);
    }
    return left;
}

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::parseFactor(
    const QString& expr, int& pos) const
{
    auto base = parseAtom(expr, pos);
    skipSpaces(expr, pos);

    if (pos < expr.size() && expr[pos] == '^') {
        ++pos;
        auto exp = parseFactor(expr, pos);
        auto node = std::make_shared<Expr>();
        node->type = ExprType::Power;
        node->left = base;
        node->right = exp;
        return node;
    }
    return base;
}

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::parseAtom(
    const QString& expr, int& pos) const
{
    skipSpaces(expr, pos);

    /* 括号 */
    if (pos < expr.size() && expr[pos] == '(') {
        ++pos;
        auto inner = parseExpr(expr, pos);
        skipSpaces(expr, pos);
        if (pos < expr.size() && expr[pos] == ')') ++pos;
        return inner;
    }

    /* 取负 */
    if (pos < expr.size() && expr[pos] == '-') {
        ++pos;
        auto operand = parseAtom(expr, pos);
        auto node = std::make_shared<Expr>();
        node->type = ExprType::Negate;
        node->left = operand;
        return node;
    }

    /* 函数: sin, cos, tan, exp, ln */
    if (pos + 2 < expr.size()) {
        QStringRef ref(&expr, pos, 3);
        if (ref == "sin" || ref == "cos" || ref == "tan" || ref == "exp") {
            QString fname = ref.toString();
            pos += 3;
            auto arg = parseAtom(expr, pos);
            auto node = std::make_shared<Expr>();
            node->name = fname;
            if (fname == "sin") node->type = ExprType::Sin;
            else if (fname == "cos") node->type = ExprType::Cos;
            else if (fname == "tan") node->type = ExprType::Tan;
            else node->type = ExprType::Exp;
            node->left = arg;
            return node;
        }
    }
    if (pos + 1 < expr.size() && expr.mid(pos, 2) == "ln") {
        pos += 2;
        auto arg = parseAtom(expr, pos);
        auto node = std::make_shared<Expr>();
        node->type = ExprType::Ln;
        node->left = arg;
        return node;
    }

    /* 数字 */
    if (pos < expr.size() && (expr[pos].isDigit() || expr[pos] == '.')) {
        int start = pos;
        while (pos < expr.size() && (expr[pos].isDigit() || expr[pos] == '.'))
            ++pos;
        auto node = std::make_shared<Expr>();
        node->type = ExprType::Constant;
        node->value = expr.mid(start, pos - start).toDouble();
        return node;
    }

    /* 变量 */
    if (pos < expr.size() && expr[pos].isLetter()) {
        int start = pos;
        while (pos < expr.size() && (expr[pos].isLetterOrNumber() || expr[pos] == '_'))
            ++pos;
        auto node = std::make_shared<Expr>();
        node->type = ExprType::Variable;
        node->name = expr.mid(start, pos - start);
        return node;
    }

    /* 默认: 常数0 */
    auto node = std::make_shared<Expr>();
    node->type = ExprType::Constant;
    node->value = 0.0;
    return node;
}

/* ── 求导 ── */

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::differentiate(
    const std::shared_ptr<Expr>& expr,
    const QString& variable) const
{
    QElapsedTimer timer;
    timer.start();

    if (!expr) return std::make_shared<Expr>();

    auto result = std::make_shared<Expr>();

    switch (expr->type) {
    case ExprType::Constant:
        result->type = ExprType::Constant;
        result->value = 0.0;
        break;

    case ExprType::Variable:
        result->type = ExprType::Constant;
        result->value = (expr->name == variable) ? 1.0 : 0.0;
        break;

    case ExprType::Add: {
        auto dl = differentiate(expr->left, variable);
        auto dr = differentiate(expr->right, variable);
        result->type = ExprType::Add;
        result->left = dl;
        result->right = dr;
        break;
    }
    case ExprType::Subtract: {
        auto dl = differentiate(expr->left, variable);
        auto dr = differentiate(expr->right, variable);
        result->type = ExprType::Subtract;
        result->left = dl;
        result->right = dr;
        break;
    }
    case ExprType::Multiply: {
        /* 乘法法则: (f*g)' = f'*g + f*g' */
        auto fprime = differentiate(expr->left, variable);
        auto gprime = differentiate(expr->right, variable);
        auto term1 = std::make_shared<Expr>();
        term1->type = ExprType::Multiply;
        term1->left = fprime;
        term1->right = expr->right;
        auto term2 = std::make_shared<Expr>();
        term2->type = ExprType::Multiply;
        term2->left = expr->left;
        term2->right = gprime;
        result->type = ExprType::Add;
        result->left = term1;
        result->right = term2;
        break;
    }
    case ExprType::Divide: {
        /* 除法法则: (f/g)' = (f'*g - f*g') / g^2 */
        auto fprime = differentiate(expr->left, variable);
        auto gprime = differentiate(expr->right, variable);
        auto num1 = std::make_shared<Expr>();
        num1->type = ExprType::Multiply;
        num1->left = fprime; num1->right = expr->right;
        auto num2 = std::make_shared<Expr>();
        num2->type = ExprType::Multiply;
        num2->left = expr->left; num2->right = gprime;
        auto numerator = std::make_shared<Expr>();
        numerator->type = ExprType::Subtract;
        numerator->left = num1; numerator->right = num2;
        auto denom = std::make_shared<Expr>();
        denom->type = ExprType::Power;
        denom->left = expr->right;
        auto two = std::make_shared<Expr>();
        two->type = ExprType::Constant; two->value = 2.0;
        denom->right = two;
        result->type = ExprType::Divide;
        result->left = numerator; result->right = denom;
        break;
    }
    case ExprType::Power: {
        /* 简化: 仅支持 f(x)^n (n为常数) => n*f^(n-1)*f' */
        auto n = std::make_shared<Expr>();
        n->type = ExprType::Constant;
        auto nVal = expr->right;
        n->value = (nVal && nVal->type == ExprType::Constant) ? nVal->value : 1.0;
        auto nMinus1 = std::make_shared<Expr>();
        nMinus1->type = ExprType::Constant;
        nMinus1->value = n->value - 1.0;
        auto powTerm = std::make_shared<Expr>();
        powTerm->type = ExprType::Power;
        powTerm->left = expr->left; powTerm->right = nMinus1;
        auto fprime = differentiate(expr->left, variable);
        auto inner = std::make_shared<Expr>();
        inner->type = ExprType::Multiply;
        inner->left = powTerm; inner->right = fprime;
        result->type = ExprType::Multiply;
        result->left = n; result->right = inner;
        break;
    }
    case ExprType::Negate: {
        auto d = differentiate(expr->left, variable);
        result->type = ExprType::Negate;
        result->left = d;
        break;
    }
    case ExprType::Sin: {
        /* d/dx sin(f) = cos(f) * f' */
        auto fprime = differentiate(expr->left, variable);
        auto cosF = std::make_shared<Expr>();
        cosF->type = ExprType::Cos; cosF->left = expr->left;
        result->type = ExprType::Multiply;
        result->left = cosF; result->right = fprime;
        break;
    }
    case ExprType::Cos: {
        /* d/dx cos(f) = -sin(f) * f' */
        auto fprime = differentiate(expr->left, variable);
        auto sinF = std::make_shared<Expr>();
        sinF->type = ExprType::Sin; sinF->left = expr->left;
        auto mul = std::make_shared<Expr>();
        mul->type = ExprType::Multiply;
        mul->left = sinF; mul->right = fprime;
        result->type = ExprType::Negate; result->left = mul;
        break;
    }
    case ExprType::Tan: {
        /* d/dx tan(f) = (1 + tan^2(f)) * f' */
        auto fprime = differentiate(expr->left, variable);
        auto tan2 = std::make_shared<Expr>();
        tan2->type = ExprType::Power; tan2->left = expr->left;
        auto two = std::make_shared<Expr>();
        two->type = ExprType::Constant; two->value = 2.0;
        tan2->right = two;
        auto one = std::make_shared<Expr>();
        one->type = ExprType::Constant; one->value = 1.0;
        auto sum = std::make_shared<Expr>();
        sum->type = ExprType::Add; sum->left = one; sum->right = tan2;
        result->type = ExprType::Multiply;
        result->left = sum; result->right = fprime;
        break;
    }
    case ExprType::Exp: {
        /* d/dx exp(f) = exp(f) * f' */
        auto fprime = differentiate(expr->left, variable);
        auto expF = std::make_shared<Expr>();
        expF->type = ExprType::Exp; expF->left = expr->left;
        result->type = ExprType::Multiply;
        result->left = expF; result->right = fprime;
        break;
    }
    case ExprType::Ln: {
        /* d/dx ln(f) = f' / f */
        auto fprime = differentiate(expr->left, variable);
        result->type = ExprType::Divide;
        result->left = fprime; result->right = expr->left;
        break;
    }
    }

    m_stats.totalDerivations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDerivations;

    return simplify(result);
}

/* ── 化简 ── */

std::shared_ptr<SymbolicDerivative::Expr> SymbolicDerivative::simplify(
    const std::shared_ptr<Expr>& expr) const
{
    if (!expr) return expr;

    auto left = expr->left ? simplify(expr->left) : nullptr;
    auto right = expr->right ? simplify(expr->right) : nullptr;

    auto result = std::make_shared<Expr>();
    result->type = expr->type;
    result->value = expr->value;
    result->name = expr->name;
    result->left = left;
    result->right = right;

    bool leftConst = left && left->type == ExprType::Constant;
    bool rightConst = right && right->type == ExprType::Constant;

    switch (expr->type) {
    case ExprType::Add:
        if (leftConst && rightConst) {
            result->type = ExprType::Constant;
            result->value = left->value + right->value;
            result->left = result->right = nullptr;
        } else if (leftConst && qFuzzyIsNull(left->value)) {
            return right;
        } else if (rightConst && qFuzzyIsNull(right->value)) {
            return left;
        }
        break;
    case ExprType::Subtract:
        if (leftConst && rightConst) {
            result->type = ExprType::Constant;
            result->value = left->value - right->value;
            result->left = result->right = nullptr;
        } else if (rightConst && qFuzzyIsNull(right->value)) {
            return left;
        }
        break;
    case ExprType::Multiply:
        if (leftConst && rightConst) {
            result->type = ExprType::Constant;
            result->value = left->value * right->value;
            result->left = result->right = nullptr;
        } else if ((leftConst && qFuzzyIsNull(left->value)) ||
                   (rightConst && qFuzzyIsNull(right->value))) {
            result->type = ExprType::Constant; result->value = 0.0;
            result->left = result->right = nullptr;
        } else if (leftConst && qFuzzyCompare(left->value, 1.0)) {
            return right;
        } else if (rightConst && qFuzzyCompare(right->value, 1.0)) {
            return left;
        }
        break;
    case ExprType::Divide:
        if (leftConst && qFuzzyIsNull(left->value)) {
            result->type = ExprType::Constant; result->value = 0.0;
            result->left = result->right = nullptr;
        } else if (leftConst && rightConst && !qFuzzyIsNull(right->value)) {
            result->type = ExprType::Constant;
            result->value = left->value / right->value;
            result->left = result->right = nullptr;
        } else if (rightConst && qFuzzyCompare(right->value, 1.0)) {
            return left;
        }
        break;
    case ExprType::Power:
        if (leftConst && rightConst) {
            result->type = ExprType::Constant;
            result->value = qPow(left->value, right->value);
            result->left = result->right = nullptr;
        } else if (rightConst && qFuzzyCompare(right->value, 1.0)) {
            return left;
        } else if (rightConst && qFuzzyIsNull(right->value)) {
            result->type = ExprType::Constant; result->value = 1.0;
            result->left = result->right = nullptr;
        }
        break;
    case ExprType::Negate:
        if (leftConst) {
            result->type = ExprType::Constant;
            result->value = -left->value;
            result->left = nullptr;
        }
        break;
    default:
        break;
    }

    m_stats.totalSimplifications++;
    return result;
}

/* ── 字符串化 ── */

QString SymbolicDerivative::toString(const std::shared_ptr<Expr>& expr) const
{
    if (!expr) return "0";

    switch (expr->type) {
    case ExprType::Constant:
        return QString::number(expr->value, 'g', 6);
    case ExprType::Variable:
        return expr->name;
    case ExprType::Add:
        return "(" + toString(expr->left) + " + " + toString(expr->right) + ")";
    case ExprType::Subtract:
        return "(" + toString(expr->left) + " - " + toString(expr->right) + ")";
    case ExprType::Multiply:
        return "(" + toString(expr->left) + " * " + toString(expr->right) + ")";
    case ExprType::Divide:
        return "(" + toString(expr->left) + " / " + toString(expr->right) + ")";
    case ExprType::Power:
        return "(" + toString(expr->left) + "^" + toString(expr->right) + ")";
    case ExprType::Negate:
        return "(-" + toString(expr->left) + ")";
    case ExprType::Sin:
        return "sin(" + toString(expr->left) + ")";
    case ExprType::Cos:
        return "cos(" + toString(expr->left) + ")";
    case ExprType::Tan:
        return "tan(" + toString(expr->left) + ")";
    case ExprType::Exp:
        return "exp(" + toString(expr->left) + ")";
    case ExprType::Ln:
        return "ln(" + toString(expr->left) + ")";
    }
    return "0";
}

QString SymbolicDerivative::deriveAndFormat(const QString& exprStr,
                                             const QString& variable) const
{
    auto ast = parse(exprStr);
    if (!ast) return "PARSE_ERROR";

    auto deriv = differentiate(ast, variable);
    QString result = toString(deriv);

    emit derivationCompleted(result);
    return result;
}

SymbolicDerivative::Stats SymbolicDerivative::stats() const
{
    return m_stats;
}

void SymbolicDerivative::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
