/**
 * @file LobattoIntegration.h
 * @brief Gauss-Lobatto求积 — 包含端点的数值积分
 *
 * 功能: 实现Gauss-Lobatto数值积分公式，积分节点包含区间端点。
 *       适用于需要在边界处采样或与有限元方法配合的场景。
 *       使用Newton迭代法计算任意阶数的Lobatto节点和权重。
 *
 * 协作: ClenshawCurtis(Chebshev求积) / GaussQuadrature(Gauss求积)
 */
#ifndef LOBATTOINTEGRATION_H
#define LOBATTOINTEGRATION_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Gauss-Lobatto求积引擎
 */
class LobattoIntegration : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIntegrations = 0;      ///< 累计积分次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit LobattoIntegration(QObject* parent = nullptr);

    /** @brief 执行Gauss-Lobatto数值积分
     *  @param f 被积函数
     *  @param a 积分下限
     *  @param b 积分上限
     *  @param n 积分节点数(≥2)
     *  @return 积分近似值 */
    double integrate(const std::function<double(double)>& f,
                     double a, double b, int n = 5);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 积分完成 @param result 积分值 @param points 节点数 */
    void integrationCompleted(double result, int points);

private:
    /** @brief 计算Lobatto节点和权重
     *  @param n 节点数
     *  @param nodes 输出节点(标准化到[-1,1])
     *  @param weights 输出权重 */
    static void computeNodesAndWeights(int n,
                                       QVector<double>& nodes,
                                       QVector<double>& weights);

    /** @brief Legendre多项式P_n(x)的值 */
    static double legendreP(int n, double x);

    /** @brief Legendre多项式导数P_n'(x) */
    static double legendrePderivative(int n, double x);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // LOBATTOINTEGRATION_H
