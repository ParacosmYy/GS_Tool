/**
 * @file MathExpressionParser.cpp
 * @brief 数学表达式解析器实现 -- 词法分析 + 递归下降解析
 *
 * 实现 tokenize() 和四个递归下降方法 (parseExpr/parseTerm/parseUnary/parsePrimary)，
 * 以及 parse()/isValid()/availableFunctions() 等公共接口。
 * 统计接口见 MathExpressionParserStats.cpp。
 */

#include "chart/math/MathExpressionParser.h"

#include <QtMath>

// ============================================================
// 构造
// ============================================================

/** @brief 构造解析器 @param parent 父对象 */
MathExpressionParser::MathExpressionParser(QObject* parent)
    : QObject(parent)
{
}

// ============================================================
// 公共接口
// ============================================================

/** @brief 解析表达式文本 @param text 表达式字符串 @return 解析结果 */
MathExpression MathExpressionParser::parse(const QString& text)
{
    ++m_totalParses;
    m_error.clear();
    m_result = MathExpression();
    m_result.expression = text;
    m_pos = 0;

    if (text.trimmed().isEmpty()) {
        setError(QStringLiteral("表达式为空"), 0);
        ++m_totalParseErrors;
        return MathExpression();
    }

    if (!tokenize(text)) {
        ++m_totalParseErrors;
        return MathExpression();
    }

    m_pos = 0;
    if (!parseExpr() || m_pos != m_tokens.size() - 1) {
        if (m_error.isEmpty()) {
            setError(QStringLiteral("意外的符号"), 0);
        }
        ++m_totalParseErrors;
        return MathExpression();
    }

    m_result.expression = text;
    return m_result;
}

/** @brief 检查表达式是否合法 @param text 表达式字符串 @return true=语法正确 */
bool MathExpressionParser::isValid(const QString& text) const
{
    auto* self = const_cast<MathExpressionParser*>(this);
    self->parse(text);
    return self->m_error.isEmpty();
}

/** @brief 获取支持的函数名列表 @return 函数名列表 */
QStringList MathExpressionParser::availableFunctions() const
{
    return QStringList()
        << QStringLiteral("abs") << QStringLiteral("sqrt")
        << QStringLiteral("log") << QStringLiteral("exp")
        << QStringLiteral("sin") << QStringLiteral("cos")
        << QStringLiteral("derivative") << QStringLiteral("integral")
        << QStringLiteral("avg") << QStringLiteral("max")
        << QStringLiteral("min") << QStringLiteral("scale")
        << QStringLiteral("offset") << QStringLiteral("lowpass")
        << QStringLiteral("highpass");
}

/** @brief 获取最近一次错误信息 @return 错误描述 */
QString MathExpressionParser::errorMessage() const
{
    return m_error;
}

// ============================================================
// 词法分析
// ============================================================

/** @brief 词法分析: 将输入拆分为Token序列 @param text 输入文本 @return true=成功 */
bool MathExpressionParser::tokenize(const QString& text)
{
    m_tokens.clear();
    const int len = text.length();
    int i = 0;

    while (i < len) {
        const QChar ch = text[i];

        // 跳过空白
        if (ch.isSpace()) {
            ++i;
            continue;
        }

        // 数值常量 (整数或小数)
        if (ch.isDigit() || (ch == QLatin1Char('.') && i + 1 < len && text[i + 1].isDigit())) {
            int start = i;
            while (i < len && (text[i].isDigit() || text[i] == QLatin1Char('.'))) {
                ++i;
            }
            Token tok;
            tok.type = TokType::Number;
            tok.text = text.mid(start, i - start);
            tok.value = tok.text.toDouble();
            tok.pos = start;
            m_tokens.append(tok);
            continue;
        }

        // 字母开头: 通道引用(ch1-ch8)或函数名
        if (ch.isLetter()) {
            int start = i;
            while (i < len && (text[i].isLetterOrNumber() || text[i] == QLatin1Char('_'))) {
                ++i;
            }
            QString word = text.mid(start, i - start).toLower();
            Token tok;
            tok.pos = start;

            if (word.startsWith(QLatin1String("ch")) && word.length() >= 3) {
                bool ok = false;
                int chNum = word.mid(2).toInt(&ok);
                if (ok && chNum >= 1 && chNum <= 8) {
                    tok.type = TokType::Channel;
                    tok.text = word;
                    tok.channel = chNum - 1; // 0-based
                    m_tokens.append(tok);
                    continue;
                }
            }

            if (availableFunctions().contains(word)) {
                tok.type = TokType::Function;
                tok.text = word;
                m_tokens.append(tok);
                ++m_totalFunctionsUsed;
                continue;
            }

            setError(QStringLiteral("未知的标识符: %1").arg(word), start);
            return false;
        }

        // 运算符和括号
        Token tok;
        tok.pos = i;
        tok.text = ch;
        switch (ch.unicode()) {
        case '+': tok.type = TokType::Plus;    break;
        case '-': tok.type = TokType::Minus;   break;
        case '*': tok.type = TokType::Star;    break;
        case '/': tok.type = TokType::Slash;   break;
        case '(': tok.type = TokType::LParen;  break;
        case ')': tok.type = TokType::RParen;  break;
        case ',': tok.type = TokType::Comma;   break;
        default:
            setError(QStringLiteral("非法字符: '%1'").arg(ch), i);
            return false;
        }
        m_tokens.append(tok);
        ++i;
    }

    // 结束标记
    Token end;
    end.type = TokType::End;
    end.pos = len;
    m_tokens.append(end);
    return true;
}

// ============================================================
// 递归下降解析
// ============================================================

/** @brief expr → term (('+' | '-') term)* */
bool MathExpressionParser::parseExpr()
{
    if (!parseTerm()) {
        return false;
    }

    while (m_pos < m_tokens.size()) {
        auto type = m_tokens[m_pos].type;
        if (type == TokType::Plus) {
            ++m_pos;
            if (!parseTerm()) return false;
            m_result.operation = MathOp::Add;
        } else if (type == TokType::Minus) {
            ++m_pos;
            if (!parseTerm()) return false;
            m_result.operation = MathOp::Subtract;
        } else {
            break;
        }
    }
    return true;
}

/** @brief term → unary (('*' | '/') unary)* */
bool MathExpressionParser::parseTerm()
{
    if (!parseUnary()) {
        return false;
    }

    while (m_pos < m_tokens.size()) {
        auto type = m_tokens[m_pos].type;
        if (type == TokType::Star) {
            ++m_pos;
            if (!parseUnary()) return false;
            m_result.operation = MathOp::Multiply;
        } else if (type == TokType::Slash) {
            ++m_pos;
            if (!parseUnary()) return false;
            m_result.operation = MathOp::Divide;
        } else {
            break;
        }
    }
    return true;
}

/** @brief unary → ('-' unary) | primary */
bool MathExpressionParser::parseUnary()
{
    if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TokType::Minus) {
        ++m_pos;
        if (!parseUnary()) return false;
        m_result.parameter = -1.0;
        m_result.operation = MathOp::Scale;
        return true;
    }
    return parsePrimary();
}

/** @brief primary → NUMBER | CHANNEL | FUNC '(' expr [',' NUMBER] ')' | '(' expr ')' */
bool MathExpressionParser::parsePrimary()
{
    if (m_pos >= m_tokens.size()) {
        setError(QStringLiteral("表达式不完整"), 0);
        return false;
    }

    const Token& tok = m_tokens[m_pos];

    switch (tok.type) {
    case TokType::Number:
        m_result.parameter = tok.value;
        m_result.operation = MathOp::Offset;
        ++m_pos;
        return true;

    case TokType::Channel:
        if (!m_result.sourceChannels.contains(tok.channel)) {
            m_result.sourceChannels.append(tok.channel);
        }
        if (m_result.sourceChannels.size() == 1) {
            // 单通道操作(可能是后续运算的左操作数)
        }
        ++m_pos;
        return true;

    case TokType::Function: {
        QString func = tok.text;
        ++m_pos; // 跳过函数名

        // 期望 '('
        if (m_pos >= m_tokens.size() || m_tokens[m_pos].type != TokType::LParen) {
            setError(QStringLiteral("函数 '%1' 后缺少 '('").arg(func), tok.pos);
            return false;
        }
        ++m_pos; // 跳过 '('

        // 解析参数表达式
        if (!parseExpr()) return false;

        // 检查可选的逗号参数 (如 scale(ch1, 0.5))
        if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TokType::Comma) {
            ++m_pos; // 跳过逗号
            // 解析第二个参数 (数值常量)
            if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TokType::Number) {
                m_result.parameter = m_tokens[m_pos].value;
                ++m_pos;
            }
        }

        // 期望 ')'
        if (m_pos >= m_tokens.size() || m_tokens[m_pos].type != TokType::RParen) {
            setError(QStringLiteral("函数 '%1' 缺少 ')'").arg(func), tok.pos);
            return false;
        }
        ++m_pos; // 跳过 ')'

        m_result.operation = functionToOp(func);
        return true;
    }

    case TokType::LParen: {
        ++m_pos; // 跳过 '('
        if (!parseExpr()) return false;

        if (m_pos >= m_tokens.size() || m_tokens[m_pos].type != TokType::RParen) {
            setError(QStringLiteral("缺少匹配的 ')'"), tok.pos);
            return false;
        }
        ++m_pos; // 跳过 ')'
        return true;
    }

    default:
        setError(QStringLiteral("意外的符号: '%1'").arg(tok.text), tok.pos);
        return false;
    }
}

// ============================================================
// 内部工具
// ============================================================

/** @brief 设置错误信息 @param msg 错误描述 @param pos 字符位置 */
void MathExpressionParser::setError(const QString& msg, int pos)
{
    m_error = QStringLiteral("位置 %1: %2").arg(pos).arg(msg);
}

/** @brief 函数名 → MathOp 映射 @param name 函数名 @return 对应运算 */
MathOp MathExpressionParser::functionToOp(const QString& name)
{
    if (name == QLatin1String("abs"))        return MathOp::Abs;
    if (name == QLatin1String("sqrt"))       return MathOp::Sqrt;
    if (name == QLatin1String("log"))        return MathOp::Log;
    if (name == QLatin1String("exp"))        return MathOp::Exp;
    if (name == QLatin1String("sin"))        return MathOp::Sin;
    if (name == QLatin1String("cos"))        return MathOp::Cos;
    if (name == QLatin1String("derivative")) return MathOp::Derivative;
    if (name == QLatin1String("integral"))   return MathOp::Integral;
    if (name == QLatin1String("avg"))        return MathOp::Average;
    if (name == QLatin1String("max"))        return MathOp::Max;
    if (name == QLatin1String("min"))        return MathOp::Min;
    if (name == QLatin1String("scale"))      return MathOp::Scale;
    if (name == QLatin1String("offset"))     return MathOp::Offset;
    if (name == QLatin1String("lowpass"))    return MathOp::LowPass;
    if (name == QLatin1String("highpass"))   return MathOp::HighPass;
    return MathOp::Abs;
}

/** @brief MathOp → 可读名称 @param op 运算类型 @return 名称 */
QString MathExpressionParser::opName(MathOp op)
{
    switch (op) {
    case MathOp::Add:        return QStringLiteral("+");
    case MathOp::Subtract:   return QStringLiteral("-");
    case MathOp::Multiply:   return QStringLiteral("*");
    case MathOp::Divide:     return QStringLiteral("/");
    case MathOp::Abs:        return QStringLiteral("abs");
    case MathOp::Sqrt:       return QStringLiteral("sqrt");
    case MathOp::Log:        return QStringLiteral("log");
    case MathOp::Exp:        return QStringLiteral("exp");
    case MathOp::Sin:        return QStringLiteral("sin");
    case MathOp::Cos:        return QStringLiteral("cos");
    case MathOp::Tan:        return QStringLiteral("tan");
    case MathOp::Derivative: return QStringLiteral("derivative");
    case MathOp::Integral:   return QStringLiteral("integral");
    case MathOp::Average:    return QStringLiteral("avg");
    case MathOp::Max:        return QStringLiteral("max");
    case MathOp::Min:        return QStringLiteral("min");
    case MathOp::Scale:      return QStringLiteral("scale");
    case MathOp::Offset:     return QStringLiteral("offset");
    case MathOp::LowPass:    return QStringLiteral("lowpass");
    case MathOp::HighPass:   return QStringLiteral("highpass");
    }
    return QStringLiteral("unknown");
}
