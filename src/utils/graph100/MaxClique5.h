#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 最大团求解器
 *
 * 基于Bron-Kerbosch算法寻找图中的最大完全子图。
 */
class MaxClique5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalCliquesFound = 0;
        int totalMaxCliqueSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaxClique5(QObject* parent = nullptr);

    /** @brief 查找最大团 */
    QVector<int> find(const QVector<QVector<int>>& adjacency);

    /** @brief 列举所有极大团 */
    QVector<QVector<int>> enumerate(const QVector<QVector<int>>& adjacency, int maxSize = 1000);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cliqueFound(int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
