#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief KD-Tree多维空间索引
 *
 * k维空间最近邻查询和范围搜索数据结构。
 */
class KDTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalNearestQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KDTree6(int dimensions = 2, QObject* parent = nullptr);

    /** @brief 插入k维点 */
    void insert(const QVector<double>& point, const QVariant& data = QVariant());

    /** @brief 最近邻查询 */
    QPair<QVector<double>, QVariant> nearestNeighbor(const QVector<double>& query) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nearestFound(int nodeId, double distance);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimensions = 2;
    struct KDNode* m_root = nullptr;
};
