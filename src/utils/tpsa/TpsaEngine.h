/**
 * @file TpsaEngine.h
 * @brief 截断幂级数代数(TPSA) — 自动微分引擎
 *
 * 功能: 通过截断幂级数表示多变量函数，实现高阶自动微分，
 *       支持加/减/乘/除/三角函数/指数/对数等运算。
 *       适用于粒子加速器光学和束流动力学仿真。
 *
 * 协作: NumericalDerivative(数值微分) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

#include <vector>
#include <map>
#include <cmath>

/**
 * @brief TPSA引擎 — 截断幂级数自动微分
 */
class TpsaEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalOperations       = 0;   ///< 累计运算次数
        quint64 totalTermsProcessed   = 0;   ///< 累计处理项数
        double  avgProcessingTimeMs   = 0.0; ///< 平均处理时间(ms)
        quint64 totalVariablesCreated = 0;   ///< 累计创建变量数
    };

    /**
     * @brief TPSA单项式索引 — 多重指标表示变量的幂次
     */
    using MonoIndex = QVector<int>;

    /**
     * @brief 截断幂级数
     */
    struct Tpsa {
        double order = 0.0;               ///< 截断阶数
        int numVars = 0;                   ///< 变量数
        std::map<MonoIndex, double> terms; ///< 单项式 → 系数

        /** @brief 获取常数部分 @return 常数项值 */
        double constant() const;

        /** @brief 获取对变量v的偏导数 @param v 变量索引 @return 偏导TPSA */
        Tpsa derivative(int v) const;

        /** @brief 评估(代入变量值) @param values 变量值 @return 函数值 */
        double evaluate(const QVector<double>& values) const;

        /** @brief 项数 @return 非零项数 */
        int termCount() const;
    };

    /**
     * @brief 构造函数
     * @param numVars 变量数
     * @param maxOrder 最大截断阶数
     * @param parent 父对象
     */
    explicit TpsaEngine(int numVars = 6, int maxOrder = 2, QObject* parent = nullptr);

    /** @brief 创建独立变量TPSA @param varIndex 变量索引 @return 变量TPSA */
    Tpsa makeVariable(int varIndex);

    /** @brief 创建常数TPSA @param value 常数值 @return 常数TPSA */
    Tpsa makeConstant(double value) const;

    /** @brief TPSA加法 @param a A @param b B @return a+b */
    Tpsa add(const Tpsa& a, const Tpsa& b) const;

    /** @brief TPSA减法 @param a A @param b B @return a-b */
    Tpsa subtract(const Tpsa& a, const Tpsa& b) const;

    /** @brief TPSA乘法 @param a A @param b B @return a*b */
    Tpsa multiply(const Tpsa& a, const Tpsa& b);

    /** @brief TPSA除法 @param a 被除数 @param b 除数 @return a/b */
    Tpsa divide(const Tpsa& a, const Tpsa& b);

    /** @brief TPSA正弦 @param a 操作数 @return sin(a) */
    Tpsa sin(const Tpsa& a);

    /** @brief TPSA余弦 @param a 操作数 @return cos(a) */
    Tpsa cos(const Tpsa& a);

    /** @brief TPSA指数 @param a 操作数 @return exp(a) */
    Tpsa exp(const Tpsa& a);

    /** @brief TPSA对数 @param a 操作数 @return ln(a) */
    Tpsa log(const Tpsa& a);

    /** @brief TPSA幂 @param base 底数 @param exponent 指数 @return base^exponent */
    Tpsa pow(const Tpsa& base, double exponent);

    /** @brief TPSA平方根 @param a 操作数 @return sqrt(a) */
    Tpsa sqrt(const Tpsa& a);

    int numVars() const { return m_numVars; }
    int maxOrder() const { return m_maxOrder; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 运算完成 @param opType 运算类型 */
    void operationCompleted(const QString& opType);

private:
    /** @brief 单项式阶数 @param mono 单项式 @return 阶数 */
    int monoOrder(const MonoIndex& mono) const;

    /** @brief 截断超阶项 @param tpsa TPSA */
    void truncate(Tpsa& tpsa) const;

    /** @brief 泰勒展开 sin(d) @param d TPSA @return sin(d)近似 */
    Tpsa taylorSin(const Tpsa& d);

    /** @brief 泰勒展开 cos(d) @param d TPSA @return cos(d)近似 */
    Tpsa taylorCos(const Tpsa& d);

    /** @brief 泰勒展开 exp(d) @param d TPSA @return exp(d)近似 */
    Tpsa taylorExp(const Tpsa& d);

    int m_numVars;   ///< 变量数
    int m_maxOrder;  ///< 最大截断阶数

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;
};
