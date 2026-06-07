/**
 * @file EdgeColoring3.h
 * @brief 边着色(Tait定理三次图+Vizing定理扩展) — Edge Coloring via Tait's Theorem for Cubic Graphs and Vizing's Theorem Extension
 *
 * 功能: 实现图的边着色算法，支持Tait定理(三次图3-边着色)、
 *       Vizing定理(D+1着色)和Misra-Kies边着色算法。
 *
 * 协作: GraphColoring9(顶点着色) / BipartiteMatch8(二部图匹配) / EulerTour7(欧拉回路)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 边着色(Tait定理三次图+Vizing定理扩展)
 */
class EdgeColoring3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int vertexCount = 0;
        int edgeCount = 0;
        int chromaticIndex = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring3(QObject *parent = nullptr);
    ~EdgeColoring3() override;

    /** @brief Color edges using best available algorithm */
    QVector<int> color(const QVector<QVector<int>>& adjacency);

    /** @brief Tait coloring for cubic (3-regular) graphs */
    QVector<int> taitColor(const QVector<QVector<int>>& adj) const;

    /** @brief Vizing extension: at most Delta+1 colors */
    QVector<int> vizingColor(const QVector<QVector<int>>& adj) const;

    /** @brief Misra-Kies edge coloring algorithm */
    QVector<int> misraKies(const QVector<QVector<int>>& adj) const;

    /** @brief Check if graph is cubic (3-regular) */
    static bool isCubic(const QVector<QVector<int>>& adj);

    /** @brief Compute maximum degree Delta */
    static int maxDegree(const QVector<QVector<int>>& adj);

    /** @brief Extract edge list from adjacency matrix */
    static QVector<QPair<int, int>> edgeList(const QVector<QVector<int>>& adj);

    /** @brief Verify coloring is valid */
    static bool verifyColoring(const QVector<QPair<int, int>>& edges,
                                const QVector<int>& colors);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colors, int edges, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build free-color list for a vertex */
    QVector<int> freeColors(int vertex, int maxColor,
                             const QVector<QVector<int>>& adj,
                             const QVector<int>& edgeColors,
                             const QVector<QPair<int, int>>& edges) const;

    /** @brief Find fan chain for Misra-Kies */
    int findFan(int v, int u, const QVector<QVector<int>>& adj,
                 const QVector<int>& edgeColors,
                 const QVector<QPair<int, int>>& edges) const;
};
