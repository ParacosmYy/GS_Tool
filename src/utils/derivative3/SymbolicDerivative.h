/**
 * @file SymbolicDerivative.h
 * @brief 符号微分 — 简单表达式解析与自动求导
 *
 * 支持基本算术运算(+,-,*,/)、幂运算(^)、三角函数(sin,cos,tan)、
 * 指数/对数(exp,ln)的符号微分。输出为化简后的表达式字符串。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <memory>

/**
 * @class SymbolicDerivative
 * @brief 符号微分 — 解析表达式树并自动求导
 *
 * 内部维护表达式树(Expr AST), 对指定变量求导后化简输出。
 */
class SymbolicDerivative : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDerivations = 0;   ///< 总求导次数
        quint64 totalSimplifications = 0; ///< 总化简次数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SymbolicDerivative(QObject* parent = nullptr);

    /**
     * @brief 解析表达式字符串为AST
     * @param expr 表达式字符串(如 "x^2 + sin(x)")
     * @return AST根节点(解析失败返回nullptr)
     */
    struct Expr;
    std::shared_ptr<Expr> parse(const QString& expr) const;

    /**
     * @brief 对指定变量求符号导数
     * @param expr 表达式AST
     * @param variable 求导变量(如 "x")
     * @return 导数AST(已化简)
     */
    std::shared_ptr<Expr> differentiate(const std::shared_ptr<Expr>& expr,
                                         const QString& variable) const;

    /**
     * @brief 化简表达式
     * @param expr 输入表达式AST
     * @return 化简后的AST
     */
    std::shared_ptr<Expr> simplify(const std::shared_ptr<Expr>& expr) const;

    /**
     * @brief 将AST转换为字符串
     * @param expr 表达式AST
     * @return 可读字符串
     */
    QString toString(const std::shared_ptr<Expr>& expr) const;

    /**
     * @brief 便捷方法: 解析+求导+化简+输出字符串
     * @param exprStr 表达式字符串
     * @param variable 求导变量
     * @return 导数字符串
     */
    QString deriveAndFormat(const QString& exprStr,
                            const QString& variable) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 表达式AST节点类型 */
    enum class ExprType {
        Constant,   ///< 常数
        Variable,   ///< 变量
        Add,        ///< 加法
        Subtract,   ///< 减法
        Multiply,   ///< 乘法
        Divide,     ///< 除法
        Power,      ///< 幂运算
        Negate,     ///< 取负
        Sin, Cos, Tan, ///< 三角函数
        Exp, Ln     ///< 指数/对数
    };

    /** @brief 表达式AST节点 */
    struct Expr {
        ExprType type = ExprType::Constant;
        double value = 0.0;             ///< 常数值
        QString name;                   ///< 变量名/函数名
        std::shared_ptr<Expr> left;     ///< 左子节点
        std::shared_ptr<Expr> right;    ///< 右子节点
    };

signals:
    /** @brief 求导完成 @param result 导数结果字符串 */
    void derivationCompleted(const QString& result);

private:
    std::shared_ptr<Expr> parseExpr(const QString& expr, int& pos) const;
    std::shared_ptr<Expr> parseTerm(const QString& expr, int& pos) const;
    std::shared_ptr<Expr> parseFactor(const QString& expr, int& pos) const;
    std::shared_ptr<Expr> parseAtom(const QString& expr, int& pos) const;
    void skipSpaces(const QString& expr, int& pos) const;

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
