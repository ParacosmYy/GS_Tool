/**
 * @file GraphIsomorphism14.h
 * @brief 图同构检测(分布式非同构证书与规范标记的高效图哈希) — Graph Isomorphism with Distributed Non-isomorphism Certificates and Canonical Labeling for Efficient Graph Hashing
 *
 * 功能: 实现图同构检测(graph isomorphism)，采用分布式非同构证书(distributed non-isomorphism certificates)
 *       与规范标记(canonical labeling)实现高效图哈希(efficient graph hashing)。
 *
 * 协作: AStarShortestPath12(A*最短路径) / DijkstraShortest11(Dijkstra最短路径) / MSTKruskal10(Kruskal MST)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 图同构检测(分布式非同构证书与规范标记)
 */
class GraphIsomorphism14 : public QObject {
    Q_OBJECT

public:
    /** @brief Graph as adjacency matrix */
    using AdjMatrix = QVector<QVector<int>>;

    /** @brief Isomorphism result */
    struct IsoResult {
        bool isomorphic = false;
        QVector<int> mapping;           // vertex mapping G1 -> G2
        QString canonicalLabel1;
        QString canonicalLabel2;
        double confidence = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int graphSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorphism14(QObject *parent = nullptr);
    ~GraphIsomorphism14() override;

    /** @brief Check if two graphs are isomorphic */
    IsoResult checkIsomorphic(const AdjMatrix& g1, const AdjMatrix& g2);

    /** @brief Compute canonical label (string hash) of a graph */
    QString canonicalLabel(const AdjMatrix& g) const;

    /** @brief Compute vertex invariant (degree sequence hash) */
    QVector<quint64> vertexInvariants(const AdjMatrix& g) const;

    /** @brief Generate non-isomorphism certificate */
    QString nonIsoCertificate(const AdjMatrix& g) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void isomorphismChecked(bool result, double confidence, double timeMs);
    void canonicalLabelComputed(const QString& label);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute degree sequence */
    QVector<int> degreeSequence(const AdjMatrix& g) const;

    /** @brief Refine vertex coloring via WL iteration */
    QVector<int> wlColoring(const AdjMatrix& g, int maxIter = 20) const;

    /** @brief Check degree sequence compatibility (quick filter) */
    bool degreeCompatible(const AdjMatrix& g1, const AdjMatrix& g2) const;

    /** @brief Find mapping via backtracking with pruning */
    bool findMapping(const AdjMatrix& g1, const AdjMatrix& g2,
                      const QVector<int>& colors1, const QVector<int>& colors2,
                      QVector<int>& mapping, QVector<bool>& used, int depth);

    /** @brief Build adjacency string for canonical form */
    QString adjacencyString(const AdjMatrix& g, const QVector<int>& order) const;

    /** @brief Hash function for vertex colors */
    quint64 hashCombine(quint64 h1, quint64 h2) const;
};
