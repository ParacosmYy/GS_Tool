/**
 * @file LaguerrePoly.h
 * @brief Laguerre多项式 — 求值与求根
 *
 * 功能: 实现Laguerre多项式的求值、递推计算和根求解。
 *       支持广义Laguerre多项式L_n^{alpha}(x)，
 *       使用Laguerre方法进行多项式根求解。
 *       适用于量子力学计算、高斯积分节点计算和信号处理。
 *
 * 协作: GaussianQuadrature(高斯积分) / HermitePoly(正交多项式)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <complex>
#include <functional>
#include <vector>

/**
 * @brief Laguerre多项式求值与求根
 */
class LaguerrePoly : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEvaluations = 0;       ///< 累计求值次数
        int totalRootFindings = 0;       ///< 累计求根次数
        int totalIntegrations = 0;       ///< 累计积分次数
        int totalRootsFound = 0;         ///< 累计找到的根数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit LaguerrePoly(QObject* parent = nullptr);

    /**
     * @brief 计算广义Laguerre多项式 L_n^{alpha}(x)
     * @param n 阶数
     * @param alpha 参数alpha
     * @param x 求值点
     * @return L_n^{alpha}(x)的值
     */
    double evaluate(int n, double alpha, double x) const;

    /**
     * @brief 批量计算Laguerre多项式(返回L_0到L_n)
     * @param n 最大阶数
     * @param alpha 参数alpha
     * @param x 求值点
     * @return L_0(x)到L_n(x)的值列表
     */
    std::vector<double> evaluateAll(int n, double alpha, double x) const;

    /**
     * @brief 计算Laguerre多项式的导数 d/dx L_n^{alpha}(x)
     * @param n 阶数
     * @param alpha 参数alpha
     * @param x 求值点
     * @return 导数值
     */
    double derivative(int n, double alpha, double x) const;

    /**
     * @brief 求Laguerre多项式L_n^{alpha}(x)的所有实根
     * @param n 阶数
     * @param alpha 参数alpha
     * @return 根的列表(升序)
     */
    std::vector<double> findRoots(int n, double alpha);

    /**
     * @brief 计算Gauss-Laguerre积分节点和权重
     * @param n 积分点数
     * @param alpha 参数alpha
     * @return (nodes, weights)对
     */
    std::pair<std::vector<double>, std::vector<double>> gaussNodesWeights(
        int n, double alpha);

    /**
     * @brief 使用Gauss-Laguerre积分计算 \int_0^inf f(x) x^alpha e^{-x} dx
     * @param n 积分点数
     * @param alpha 参数alpha
     * @param f 被积函数
     * @return 积分值
     */
    double integrate(int n, double alpha, const std::function<double(double)>& f);

    /**
     * @brief 计算三项递推系数
     * @param n 阶数
     * @param alpha 参数alpha
     * @return (a_n, b_n, c_n) 使得 L_n = a_n * L_{n-1} + b_n * x * L_{n-1} + c_n * L_{n-2}
     */
    std::tuple<double, double, double> recurrenceCoeffs(int n, double alpha) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求根完成 @param n 阶数 @param rootCount 根数 */
    void rootsFound(int n, int rootCount);

    /** @brief 积分完成 @param n 积分点数 @param value 积分值 */
    void integrationCompleted(int n, double value);

private:
    /**
     * @brief Laguerre方法求解一般多项式的单个根
     * @param n 阶数
     * @param alpha 参数alpha
     * @param initialGuess 初始猜测
     * @param maxIter 最大迭代
     * @return 根
     */
    double laguerreRoot(int n, double alpha, double initialGuess, int maxIter = 100) const;

    mutable Stats m_stats;                  ///< 统计信息
    mutable double m_timeSum = 0.0;         ///< 累计耗时
};
