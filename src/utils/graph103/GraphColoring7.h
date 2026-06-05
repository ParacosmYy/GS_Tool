#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图着色算法工具类
 *
 * 提供图着色求解功能，支持设置顶点数和边，
 * 计算最小着色数及对应着色方案。
 */
class GraphColoring7 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit GraphColoring7(QObject* parent = nullptr);

    /** @brief 设置图的顶点数量 */
    void setVertexCount(int count);

    /** @brief 添加一条无向边 */
    void addEdge(int from, int to);

    /** @brief 执行图着色求解 */
    void solve();

    /** @brief 获取色数(最小着色颜色数) */
    int chromaticNumber() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回色数 */
    void solved(int colors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    QVector<QPair<int, int>> m_edges;
    int m_chromaticNumber = 0;
};
