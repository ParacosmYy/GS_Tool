/**
 * @file ClenshawCurtis.h
 * @brief Clenshaw-Curtis求积 — Chebyshev节点数值积分
 *
 * 功能: 实现Clenshaw-Curtis数值积分公式，使用Chebyshev节点
 *       (cos(k*pi/(n-1)))作为积分节点。通过DCT-II变换高效计算权重。
 *       适用于光滑函数的高精度积分，误差收敛速度接近Gauss求积。
 *
 * 协作: LobattoIntegration(包含端点求积) / GaussQuadrature(Gauss求积)
 */
#ifndef CLENSHAWCURTIS_H
#define CLENSHAWCURTIS_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Clenshaw-Curtis求积引擎
 */
class ClenshawCurtis : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIntegrations = 0;      ///< 累计积分次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit ClenshawCurtis(QObject* parent = nullptr);

    /** @brief 执行Clenshaw-Curtis数值积分
     *  @param f 被积函数
     *  @param a 积分下限
     *  @param b 积分上限
     *  @param n 积分节点数(≥2，推荐奇数)
     *  @return 积分近似值 */
    double integrate(const std::function<double(double)>& f,
                     double a, double b, int n = 17);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 积分完成 @param result 积分值 @param points 节点数 */
    void integrationCompleted(double result, int points);

private:
    /** @brief 计算Clenshaw-Curtis权重
     *  @param n 节点数
     *  @return 权重数组 */
    static QVector<double> computeWeights(int n);

    /** @brief 计算Chebyshev节点(cos变换)
     *  @param n 节点数
     *  @return 节点数组(标准化到[-1,1]) */
    static QVector<double> computeNodes(int n);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // CLENSHAWCURTIS_H
