#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Householder QR分解
 *
 * 使用Householder反射进行稳定的QR分解。
 */
class HouseholderQR3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDecompositions = 0;
        int totalSolves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HouseholderQR3(QObject* parent = nullptr);

    /** @brief QR分解 */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief 求解线性系统 */
    QVector<double> solve(const QVector<double>& rhs) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rows, int cols, double conditioning);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_Q;
    QVector<QVector<double>> m_R;
};
