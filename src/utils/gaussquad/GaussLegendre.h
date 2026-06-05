/**
 * @file GaussLegendre.h
 * @brief Gauss-Legendre数值积分引擎 — 高精度数值积分
 *
 * 功能: 使用Gauss-Legendre求积公式对函数在有限区间上进行数值积分。
 *       支持2~5点求积，对应3~9阶多项式精确积分。
 *       内置Legendre多项式根和权重的查表。
 *
 * 协作: RichardsonExtrapolation(精度提升) / SpectrumAnalyzer(频谱积分)
 */
#ifndef GAUSSLEGENDRE_H
#define GAUSSLEGENDRE_H

#include <QObject>
#include <functional>

/**
 * @brief Gauss-Legendre数值积分引擎
 */
class GaussLegendre : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIntegrations = 0;      ///< 累计积分次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit GaussLegendre(QObject* parent = nullptr);

    /**
     * @brief Gauss-Legendre数值积分
     * @param f 被积函数
     * @param a 积分下限
     * @param b 积分上限
     * @param n 求积点数(2~5)
     * @return 积分近似值
     *
     * 将积分区间[a,b]映射到[-1,1]后应用Gauss-Legendre求积。
     * n=2精确到3次多项式，n=5精确到9次多项式。
     */
    double integrate(std::function<double(double)> f,
                     double a, double b, int n = 3);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 积分完成信号 @param result 积分结果 @param points 求积点数 */
    void integrationCompleted(double result, int points);

private:
    /**
     * @brief 获取求积节点和权重
     * @param n 点数(2~5)
     * @return QPair(节点向量, 权重向量) 节点和权重在[-1,1]上
     */
    QPair<QVector<double>, QVector<double>> getNodesAndWeights(int n);

    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // GAUSSLEGENDRE_H
