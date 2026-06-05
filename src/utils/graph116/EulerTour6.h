#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉回路/欧拉路径求解器
 *
 * 判定图中是否存在欧拉回路或欧拉路径，并构造具体的遍历序列，
 * 基于Hierholzer算法实现O(E)时间复杂度。
 */
class EulerTour6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit EulerTour6(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 寻找欧拉回路/路径 */
    QVector<int> findTour();

    /** @brief 判断图是否具有欧拉性质 */
    bool isEulerian();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 欧拉回路搜索完成信号 */
    void found(bool hasEulerTour);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
