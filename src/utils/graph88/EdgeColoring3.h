#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief EdgeColoring3 - 图边着色算法
 *
 * 使用Vizing定理进行边着色，支持简单图和多重图，
 * 返回最小或近似最小颜色数的边着色方案。
 */
class EdgeColoring3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalGraphsColored = 0;
        int totalColorsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit EdgeColoring3(QObject* parent = nullptr);

    /** @brief 对邻接表表示的图执行边着色 */
    QVector<QVector<int>> color(const QVector<QVector<int>>& adjacency, int vertexCount);

    /** @brief 获取着色使用的颜色数 */
    int chromaticIndex() const;

    /** @brief 验证着色方案的合法性 */
    bool validateColoring(const QVector<QVector<int>>& coloring) const;

    /** @brief 获取指定颜色的所有边 */
    QVector<QPair<int,int>> edgesOfColor(int color) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_chromaticIndex = 0;
    QVector<QVector<QPair<int,int>>> m_colorEdges;
};
