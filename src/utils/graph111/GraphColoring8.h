#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 图着色求解器
 *
 * 对无向图进行顶点着色，使相邻顶点颜色不同，
 * 支持贪心与回溯策略，输出色数和着色方案。
 */
class GraphColoring8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalVertices = 0;       ///< 顶点总数
        int totalEdges = 0;          ///< 边总数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit GraphColoring8(QObject* parent = nullptr);

    /** @brief 设置顶点数量 */
    void setVertexCount(int count);
    /** @brief 添加无向边 */
    void addEdge(int u, int v);
    /** @brief 执行着色求解 */
    void solve();
    /** @brief 获取所需最少颜色数 */
    int chromaticNumber() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回使用的颜色数 */
    void solved(int colorsUsed);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_vertexCount = 0;
    int m_chromaticNumber = 0;
    QVector<QPair<int, int>> m_edges;
};
