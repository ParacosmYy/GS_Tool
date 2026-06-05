#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 欧拉回路求解器
 *
 * 判定图中是否存在欧拉回路/路径,并构造完整的欧拉遍历序列,
 * 适用于DNA片段拼接、路线规划与邮递员问题。
 */
class EulerTour5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit EulerTour5(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加无向边 */
    void addEdge(int from, int to);

    /** @brief 寻找欧拉回路/路径 */
    void findTour();

    /** @brief 判断图是否为欧拉图 */
    void isEulerian();

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 欧拉回路查找结果信号 */
    void found(bool hasEulerTour);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
};
