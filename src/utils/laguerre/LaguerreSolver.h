/**
 * @file LaguerreSolver.h
 * @brief Laguerre方法求多项式根 — 高效多项式根求解
 *
 * 功能: 使用Laguerre迭代法求多项式的单个根，
 *       通过缩减(deflation)技术求全部根。
 *       Laguerre法对实数根有三次收敛速度。
 *
 * 协作: BisectionSolver(一般函数根) / ConjugateGradient(线性系统)
 */
#ifndef LAGUERRESOLVER_H
#define LAGUERRESOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Laguerre方法多项式根求解器
 *
 * 多项式以系数向量表示: coeffs[0]*x^n + ... + coeffs[n]
 * Laguerre法为单根提供三次收敛，结合deflation求全部根。
 */
class LaguerreSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息结构 */
    struct Stats {
        quint64 totalSolves = 0;        ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit LaguerreSolver(QObject* parent = nullptr);

    /**
     * @brief 用Laguerre法求多项式的一个根
     * @param coeffs 多项式系数(降幂排列)，长度 >= 2
     * @param x0 初始猜测值
     * @param maxIter 最大迭代次数，默认100
     * @param tol 容差，默认1e-14
     * @return 近似根
     */
    double solve(const QVector<double>& coeffs,
                 double x0 = 0.0,
                 int maxIter = 100,
                 double tol = 1e-14);

    /**
     * @brief 通过deflation求多项式全部根
     * @param coeffs 多项式系数(降幂排列)，长度 >= 2
     * @return 全部近似根(可能含复数根的实部)
     */
    QVector<double> solveAll(const QVector<double>& coeffs);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /**
     * @brief 找到根信号
     * @param root 近似根
     * @param iterations 实际迭代次数
     */
    void rootFound(double root, int iterations);

private:
    /**
     * @brief 求值多项式及其一阶、二阶导数
     * @param coeffs 多项式系数
     * @param x 求值点
     * @param[out] p 函数值
     * @param[out] dp 一阶导数值
     * @param[out] ddp 二阶导数值
     */
    void evalPoly(const QVector<double>& coeffs, double x,
                  double& p, double& dp, double& ddp) const;

    /**
     * @brief 多项式缩减(去除已知根)
     * @param coeffs 原多项式系数
     * @param root 已知根
     * @return 缩减后的多项式系数
     */
    QVector<double> deflate(const QVector<double>& coeffs,
                            double root) const;

    Stats m_stats;      ///< 统计信息
    double m_timeSum;   ///< 处理时间累加器
};

#endif // LAGUERRESOLVER_H
