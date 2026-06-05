#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SchurDecomp6 - Schur分解求解器
 *
 * 将矩阵分解为Q^T*A*Q = T形式，T为上三角矩阵，
 * 用于计算特征值和求解矩阵指数等矩阵函数。
 */
class SchurDecomp6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalDecompositions = 0;
        int totalEigenvalues = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomp6(QObject* parent = nullptr);

    /** @brief 执行实Schur分解 */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief 获取Schur形式的上三角矩阵T */
    QVector<QVector<double>> schurForm() const;

    /** @brief 获取正交矩阵Q */
    QVector<QVector<double>> unitaryMatrix() const;

    /** @brief 从Schur形式提取特征值 */
    QVector<std::complex<double>> eigenvalues() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, bool converged);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 0;
    QVector<QVector<double>> m_T;
    QVector<QVector<double>> m_Q;
};
