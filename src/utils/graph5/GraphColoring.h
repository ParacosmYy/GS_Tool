/**
 * @file GraphColoring.h
 * @brief 图顶点着色引擎 — 贪心+回溯算法
 *
 * 提供无向图的顶点着色算法, 支持贪心近似和回溯精确两种策略,
 * 适用于寄存器分配、频率规划、任务调度等图着色问题场景。
 */
#ifndef GRAPHCOLORING_H
#define GRAPHCOLORING_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class GraphColoring
 * @brief 图顶点着色 — 贪心近似+回溯精确解
 *
 * 典型用法:
 * @code
 *   GraphColoring gc;
 *   gc.addEdge(0, 1);
 *   gc.addEdge(1, 2);
 *   auto greedy = gc.colorGreedy();
 *   auto exact = gc.colorBacktrack(3);
 * @endcode
 */
class GraphColoring : public QObject {
    Q_OBJECT

public:
    /** @brief 着色结果结构 */
    struct ColoringResult {
        QVector<int> colors;    ///< 每个顶点的颜色编号(-1表示未着色)
        int colorCount = 0;     ///< 使用的颜色总数
        bool valid = false;     ///< 着色是否成功(合法)
    };

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalColorings = 0;    ///< 总着色次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit GraphColoring(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~GraphColoring() override;

    // ── 图构建 ──

    /**
     * @brief 添加无向边
     * @param u 顶点u编号
     * @param v 顶点v编号
     */
    void addEdge(int u, int v);

    /** @brief 清空图数据 */
    void clear();

    // ── 着色算法 ──

    /**
     * @brief 贪心着色(按度数降序Welsh-Powell策略)
     * @return 着色结果
     */
    ColoringResult colorGreedy();

    /**
     * @brief 回溯精确着色
     * @param maxColors 最大可用颜色数
     * @return 着色结果(valid=false表示无解)
     */
    ColoringResult colorBacktrack(int maxColors);

    // ── 查询 ──

    /** @brief 获取顶点数量 */
    int vertexCount() const;

    /** @brief 获取边数量 */
    int edgeCount() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 着色完成信号 @param colorCount 使用颜色数 @param valid 是否合法 */
    void coloringCompleted(int colorCount, bool valid);

private:
    /** @brief 检查顶点v使用颜色c是否合法 */
    bool isSafe(int v, int c, const QVector<int>& colors) const;

    /** @brief 回溯递归 */
    bool backtrackHelper(QVector<int>& colors, int vertex, int maxColors);

    /** @brief 邻接表 */
    QVector<QVector<int>> m_adj;

    /** @brief 边列表(用于edgeCount) */
    QVector<QPair<int, int>> m_edges;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // GRAPHCOLORING_H
