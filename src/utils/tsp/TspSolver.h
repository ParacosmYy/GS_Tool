/**
 * @file TspSolver.h
 * @brief 旅行商问题 (TSP) 求解器
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 使用最近邻启发式构造初始解，再用 2-opt 局部搜索改进。
 * 适用于中小规模(<=500城市)的距离矩阵。
 * 求解结果为近似最优，非精确解。
 */

#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <limits>
#include <random>

/**
 * @class TspSolver
 * @brief TSP 求解器 (Nearest Neighbor + 2-opt)
 *
 * 输入距离矩阵，返回近似最短路径及其总距离。
 */
class TspSolver : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalSolves = 0;       ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造 TSP 求解器 @param parent 父对象 */
    explicit TspSolver(QObject *parent = nullptr);

    /**
     * @brief 求解 TSP
     * @param distMatrix n x n 距离矩阵(对称或非对称)
     * @return QPair<总距离, 路径(城市索引序列)>
     *         路径为闭合回路: tour[0] -> tour[1] -> ... -> tour[0]
     *         失败返回 (0.0, {})
     */
    QPair<double, QVector<int>> solve(const QVector<QVector<double>> &distMatrix);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param cities 城市数量 @param totalDistance 总距离 */
    void solveCompleted(int cities, double totalDistance);

private:
    /**
     * @brief 最近邻启发式构造初始路径
     * @param distMatrix 距离矩阵
     * @param startCity 起始城市索引
     * @return 路径序列
     */
    static QVector<int> nearestNeighbor(const QVector<QVector<double>> &distMatrix,
                                        int startCity);

    /**
     * @brief 2-opt 局部搜索改进
     * @param distMatrix 距离矩阵
     * @param tour 初始路径(会被修改)
     * @return 改进后的总距离
     */
    static double twoOptImprove(const QVector<QVector<double>> &distMatrix,
                                QVector<int> &tour);

    /**
     * @brief 计算闭合路径总距离
     * @param distMatrix 距离矩阵
     * @param tour 路径序列
     * @return 总距离
     */
    static double tourDistance(const QVector<QVector<double>> &distMatrix,
                               const QVector<int> &tour);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // TSPSOLVER_H
