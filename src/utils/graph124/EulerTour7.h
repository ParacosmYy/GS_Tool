#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉回路/路径算法实现
 *
 * 在图中查找经过每条边恰好一次的路径或回路，支持有向图和无向图，
 * 适用于DNA序列拼接、邮递员问题和电路板布线优化。
 */
class EulerTour7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit EulerTour7(QObject* parent = nullptr);

    /** @brief 设置为有向图模式，否则默认无向图 */
    void setDirected(bool directed);

    /** @brief 添加一条边(from, to)，支持平行边 */
    void addEdge(int from, int to);

    /** @brief 判断当前图是否存在欧拉回路 */
    bool hasEulerCircuit() const;

    /** @brief 查找欧拉路径/回路，返回顶点访问序列 */
    QVector<int> findEulerTour();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 欧拉回路查找完成信号，返回路径长度 */
    void tourFound(int pathLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    bool m_directed = false;
};
