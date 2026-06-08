/**
 * @file GraphColoring8.h
 * @brief 图着色(递归最大优先+饱和度回跳紧色数界限) — Graph Coloring with Recursive Largest First and Saturation-based Backjumping for Tight Chromatic Number Bounds
 *
 * 功能: 实现图着色(Graph Coloring)算法，采用递归最大优先(RLF)启发式，
 *       结合饱和度(DSATUR)排序与回跳(backjumping)剪枝，获取紧凑色数上界。
 *
 * 协作: GraphIsomorphism7(图同构) / MaxClique9(最大团) / ShortestPath5(最短路径)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 图着色(RLF+饱和度回跳)
 */
class GraphColoring8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numVertices = 0;
        int numEdges = 0;
        int chromaticUsed = 0;
        int backjumps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring8(QObject *parent = nullptr);
    ~GraphColoring8() override;

    /** @brief Load adjacency matrix (symmetric, 0/1) */
    bool loadGraph(const QVector<QVector<int>>& adjacency);

    /** @brief Color graph using RLF + DSATUR backjumping */
    QVector<int> colorRLF();

    /** @brief Get lower bound via clique number */
    int cliqueLowerBound() const;

    /** @brief Verify coloring validity */
    bool verifyColoring(const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colors, int backjumps, double timeMs);

private:
    int m_n = 0;
    QVector<QVector<int>> m_adj;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute saturation degrees for uncolored vertices */
    QVector<int> computeSaturation(const QVector<int>& colors) const;

    /** @brief Select next vertex by DSATUR (max saturation, then max degree) */
    int selectDSATUR(const QVector<int>& colors,
                     const QVector<bool>& colored) const;

    /** @brief RLF phase: find uncolored vertices not adjacent to current color class */
    QVector<int> rlfPhase(const QVector<bool>& colored) const;

    /** @brief Backjumping: find most recent conflicting assignment */
    int findConflict(int vertex, int color,
                     const QVector<int>& colors) const;
};

