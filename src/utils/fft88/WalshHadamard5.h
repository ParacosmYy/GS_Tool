#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Walsh-Hadamard变换
 *
 * 快速Walsh-Hadamard变换(FWHT)，O(n log n)无乘法运算。
 */
class WalshHadamard5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalElementsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WalshHadamard5(QObject* parent = nullptr);

    /** @brief 正变换 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief 逆变换(归一化) */
    QVector<double> inverse(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double energy);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
