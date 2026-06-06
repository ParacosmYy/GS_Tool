/**
 * @file EdgeColoring.h
 * @brief 边着色(Misra-Vries边增广+Vizing定理界) — Edge Coloring via Misra-Vries Edge Augmentation with Vizing's Theorem Bounds
 *
 * 功能: 实现Misra-Vries边着色算法，支持Vizing定理上下界(Delta <= chi' <= Delta+1)、
 *       自由颜色链搜索和边增广翻转操作。
 *
 * 协作: GraphColoring6(顶点着色) / BipartiteMatching5(二分匹配) / ChromaticPolynomial4(色多项式)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 边着色器(Misra-Vries边增广)
 */
class EdgeColoring : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalColorings = 0;
        int numVertices = 0;
        int numEdges = 0;
        int maxDegree = 0;
        int numColors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring(QObject *parent = nullptr);
    ~EdgeColoring() override;

    /** @brief 对图执行边着色，返回每条边的颜色编号 */
    QVector<int> color(int n, const QVector<QPair<int, int>>& edges);

    /** @brief 计算Vizing下界(最大度数Delta) */
    int vizingLowerBound(int n, const QVector<QPair<int, int>>& edges) const;

    /** @brief 计算Vizing上界(Delta+1) */
    int vizingUpperBound(int n, const QVector<QPair<int, int>>& edges) const;

    /** @brief 检查着色是否合法 */
    bool isValid(int n, const QVector<QPair<int, int>>& edges,
                 const QVector<int>& colors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int numEdges, int numColors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find free color at vertex v (not used by any adjacent edge) */
    int findFreeColor(int v, int n, int maxColor,
                       const QVector<QVector<int>>& edgeColors) const;

    /** @brief Find maximal fan starting at vertex u, centered at vertex v */
    QVector<int> buildFan(int u, int v, int n,
                           const QVector<QVector<int>>& adj,
                           const QVector<QVector<int>>& edgeColors,
                           const QVector<int>& edgeColor) const;

    /** @brief Rotate fan: shift colors along fan */
    void rotateFan(const QVector<int>& fan, int v,
                    QVector<int>& edgeColor,
                    QVector<QVector<int>>& edgeColors);

    /** @brief Invert CD-path (flip colors along path) */
    void invertPath(int start, int c1, int c2, int n,
                     const QVector<QVector<int>>& adj,
                     QVector<int>& edgeColor,
                     QVector<QVector<int>>& edgeColors) const;
};
