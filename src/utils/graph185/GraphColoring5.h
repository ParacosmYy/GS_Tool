/**
 * @file GraphColoring5.h
 * @brief 图着色(DSATUR启发式+回溯精确色数) — Graph Coloring with DSATUR Heuristic and Backtracking for Exact Chromatic Number
 *
 * 功能: 实现图着色算法，先以DSATUR启发式快速求解，再用回溯搜索
 *       验证精确色数，支持度饱和度优先策略和剪枝优化。
 *
 * 协作: BFS6(BFS遍历) / TopologicalSort4(拓扑排序) / MaxClique3(最大团)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色求解器
 */
class GraphColoring5 : public QObject {
    Q_OBJECT

public:
    /** @brief 求解结果 */
    struct Result {
        QVector<int> colors;     ///< 每个顶点颜色(0-based)
        int numColors = 0;       ///< 使用颜色数
        bool isExact = false;    ///< 是否精确解
        quint64 backtracks = 0;  ///< 回溯次数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastColors = 0;               ///< 最近颜色数
    };

    explicit GraphColoring5(QObject *parent = nullptr);
    ~GraphColoring5() override;

    /** @brief 设置邻接表 */
    void setGraph(const QVector<QVector<int>>& adjacency);

    /** @brief DSATUR启发式着色 */
    Result solveDSATUR();

    /** @brief 回溯法求精确色数 */
    Result solveExact(int upperBound = 0);

    /** @brief 验证着色合法性 */
    bool validate(const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int colors, bool exact);

private:
    /** @brief 计算顶点饱和度(相邻不同颜色数) */
    int saturation(int vertex, const QVector<int>& colors) const;

    /** @brief 选择DSATUR下一个顶点 */
    int selectDSATURVertex(const QVector<int>& colors,
                           const QVector<bool>& colored) const;

    /** @brief 回溯递归 */
    bool backtrack(QVector<int>& colors, int vertex, int maxColors,
                   Result& best, int numColored);

    /** @brief 选择下一个未着色顶点(MRV启发式) */
    int selectVertex(const QVector<int>& colors) const;

    /** @brief 检查颜色是否可用于顶点 */
    bool canColor(int vertex, int color, const QVector<int>& colors) const;

    QVector<QVector<int>> m_adj;
    int m_numVertices = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
