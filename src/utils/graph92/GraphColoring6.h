#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GraphColoring6 - 图顶点着色算法
 *
 * 使用回溯法、贪心策略和DSATUR启发式进行
 * 顶点着色，寻找图的色数或近似最优着色。
 */
class GraphColoring6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalGraphsColored = 0;
        int totalColorsUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphColoring6(QObject* parent = nullptr);

    /** @brief 设置算法: greedy/dsatur/backtrack */
    void setAlgorithm(const QString& algorithm);

    /** @brief 对邻接表表示的图着色 */
    QVector<int> color(const QVector<QVector<int>>& adjacency);

    /** @brief 获取着色使用的颜色数 */
    int numColors() const;

    /** @brief 验证着色合法性 */
    bool isValid(const QVector<int>& coloring, const QVector<QVector<int>>& adjacency) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void coloringCompleted(int colorCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_numColors = 0;
    QString m_algorithm = "dsatur";
};
