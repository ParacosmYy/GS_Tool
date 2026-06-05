#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 桥边检测器
 *
 * 基于Tarjan算法检测无向图中的桥边(割边)。
 */
class BridgeDetect4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBridgesFound = 0;
        int totalGraphsScanned = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BridgeDetect4(QObject* parent = nullptr);

    /** @brief 查找所有桥边 */
    QVector<QPair<int, int>> findBridges(const QVector<QVector<int>>& adjacency);

    /** @brief 检查指定边是否为桥 */
    bool isBridge(const QVector<QVector<int>>& adjacency, int u, int v) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bridgeFound(int u, int v);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
