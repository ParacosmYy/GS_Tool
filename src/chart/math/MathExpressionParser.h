/**
 * @file MathExpressionParser.h
 * @brief 数学表达式解析器 -- 将文本表达式转为MathExpression结构
 *
 * 支持: ch1-ch8通道引用, +, -, *, /, (, ),
 *       函数: abs, sqrt, log, exp, sin, cos, derivative, integral, avg
 * 使用递归下降解析，提供位置相关的错误信息。
 */

#ifndef MATHEXPRESSIONPARSER_H
#define MATHEXPRESSIONPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "chart/math/MathTypes.h"

/**
 * @class MathExpressionParser
 * @brief 数学表达式文本解析器
 *
 * 将用户输入的表达式字符串(如 "ch1 + ch2", "abs(ch3)", "derivative(ch1)")
 * 解析为 MathExpression 结构体，供 WaveformMath 执行计算。
 *
 * 文法:
 *   expr   → term (('+' | '-') term)*
 *   term   → unary (('*' | '/') unary)*
 *   unary  → ('-' unary) | primary
 *   primary→ NUMBER | CHANNEL | FUNC '(' expr ')' | '(' expr ')'
 */
class MathExpressionParser : public QObject {
    Q_OBJECT

public:
    /** @brief 构造解析器 @param parent 父对象 */
    explicit MathExpressionParser(QObject* parent = nullptr);

    /**
     * @brief 解析表达式文本
     * @param text 表达式字符串 (如 "ch1 + ch2 * 0.5")
     * @return 解析结果; operation==MathOp::Add且sourceChannels为空表示解析失败
     */
    MathExpression parse(const QString& text);

    /**
     * @brief 检查表达式是否合法
     * @param text 表达式字符串
     * @return true=语法正确
     */
    bool isValid(const QString& text) const;

    /**
     * @brief 获取支持的函数名列表
     * @return 函数名列表 (小写)
     */
    QStringList availableFunctions() const;

    /**
     * @brief 获取最近一次 parse/isValid 的错误信息
     * @return 错误描述; 无错误时返回空字符串
     */
    QString errorMessage() const;

    // ── 统计 ──

    /** @brief 获取总解析次数 */
    quint64 totalParses() const;

    /** @brief 获取解析失败次数 */
    quint64 totalParseErrors() const;

    /** @brief 获取使用函数调用的总次数(累计) */
    quint64 totalFunctionsUsed() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private:
    /**
     * @brief 词法分析: 将输入文本拆分为 Token 序列
     * @param text 输入文本
     * @return true=词法分析成功
     */
    bool tokenize(const QString& text);

    /** @brief 递归下降: expr → term (('+' | '-') term)* */
    bool parseExpr();

    /** @brief 递归下降: term → unary (('*' | '/') unary)* */
    bool parseTerm();

    /** @brief 递归下降: unary → ('-' unary) | primary */
    bool parseUnary();

    /** @brief 递归下降: primary → NUMBER | CHANNEL | FUNC '(' expr ')' | '(' expr ')' */
    bool parsePrimary();

    /** @brief 设置错误信息 @param msg 错误描述 @param pos 字符位置 */
    void setError(const QString& msg, int pos);

    /** @brief 函数名 → MathOp 映射 @param name 函数名 @return 对应运算; 无匹配返回 MathOp::Abs */
    static MathOp functionToOp(const QString& name);

    /** @brief MathOp → 运算符名称 @param op 运算类型 @return 可读名称 */
    static QString opName(MathOp op);

    // ── Token 结构 ──
    enum class TokType {
        Number,    ///< 数值常量
        Channel,   ///< 通道引用 (ch1-ch8)
        Function,  ///< 函数名
        Plus,      ///< +
        Minus,     ///< -
        Star,      ///< *
        Slash,     ///< /
        LParen,    ///< (
        RParen,    ///< )
        Comma,     ///< ,
        End        ///< 输入结束
    };

    struct Token {
        TokType type = TokType::End; ///< Token类型
        QString text;                ///< 原始文本
        double value = 0.0;          ///< 数值(仅Number类型)
        int channel = -1;            ///< 通道索引(仅Channel类型)
        int pos = 0;                 ///< 在输入中的字符位置
    };

    QVector<Token> m_tokens;           ///< Token序列
    int m_pos = 0;                     ///< 当前Token索引
    QString m_error;                   ///< 最近一次错误信息
    MathExpression m_result;           ///< 解析中间结果

    // 统计计数器
    quint64 m_totalParses = 0;         ///< 总解析次数
    quint64 m_totalParseErrors = 0;    ///< 解析失败次数
    quint64 m_totalFunctionsUsed = 0;  ///< 函数调用总次数
};

#endif // MATHEXPRESSIONPARSER_H
