#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MinSpanningTree8 - 最小生成树求解器
 *
 * 支持Kruskal和Prim算法求解加权无向图的最小生成树，
 * 可处理连通和非连通图(最小生成森林)。
 */
class MinSpanningTree8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTreesComputed = 0;
        int totalEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MinSpanningTree8(QObject* parent = nullptr);

    /** @brief 设置算法: kruskal 或 prim */
    void setAlgorithm(const QString& algorithm);

    /** @brief 设置带权边列表并求解 */
    QVector<QPair<int,int>> solve(int vertexCount,
        const QVector<QPair<QPair<int,int>,double>>& edges);

    /** @brief 获取生成树总权重 */
    double totalWeight() const;

    /** @brief 检查图是否连通 */
    bool isConnected() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeComputed(int edgeCount, double weight);

private:
    /** @brief Kruskal算法求解 */
    QVector<QPair<int,int>> kruskalSolve(int n,
        const QVector<QPair<QPair<int,int>,double>>& edges);

    /** @brief Prim算法求解 */
    QVector<QPair<int,int>> primSolve(int n,
        const QVector<QPair<QPair<int,int>,double>>& edges);

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_totalWeight = 0.0;
    bool m_connected = false;
    QString m_algorithm = "kruskal";
};
