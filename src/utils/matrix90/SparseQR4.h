#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 稀疏QR分解
 *
 * 针对稀疏矩阵的高效QR分解，利用填充减少排序。
 */
class SparseQR4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFactorizations = 0;
        int totalNonzeros = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseQR4(QObject* parent = nullptr);

    /** @brief 稀疏QR分解 */
    bool factorize(const QVector<QVector<double>>& sparseMatrix);

    /** @brief 求解最小二乘问题 */
    QVector<double> solve(const QVector<double>& rhs) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int rows, int cols, double fillRatio);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_R;
};
