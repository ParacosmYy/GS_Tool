#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 对称矩阵特征值分解
 *
 * Jacobi迭代法求解对称矩阵的全部特征值和特征向量。
 */
class SymmetricEigen10 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDecompositions = 0;
        int totalRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SymmetricEigen10(QObject* parent = nullptr);

    /** @brief 计算特征值和特征向量 */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief 获取特征值(升序排列) */
    QVector<double> eigenvalues() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int dimension, int rotations);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_eigenvalues;
};
