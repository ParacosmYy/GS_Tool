#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Kronecker积矩阵运算
 *
 * 计算两个矩阵的Kronecker积，支持分块计算大矩阵。
 */
class KroneckerProduct3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalProducts = 0;
        int totalElementsComputed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KroneckerProduct3(QObject* parent = nullptr);

    /** @brief 计算两个矩阵的Kronecker积 */
    QVector<QVector<double>> compute(const QVector<QVector<double>>& a,
                                      const QVector<QVector<double>>& b);

    /** @brief 向量形式Kronecker积(展平输出) */
    QVector<double> computeFlat(const QVector<double>& a, const QVector<double>& b);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void productComputed(int rowsA, int colsA, int rowsB, int colsB);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
