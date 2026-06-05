/**
 * @file QrDecomposition.h
 * @brief QR分解 — 矩阵分解与线性方程组求解
 *
 * 功能: 基于Householder反射的QR分解，支持矩阵求逆、
 *       最小二乘求解、行列式计算，统计分解次数/耗时。
 */
#ifndef QRDECOMPOSITION_H
#define QRDECOMPOSITION_H

#include <QObject>
#include <QVector>

class QrDecomposition : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalDecompositions = 0;
        quint64 totalSolves = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit QrDecomposition(QObject* parent = nullptr);

    /** @brief QR分解 @param A 矩阵(rows×cols) @return {Q, R} */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
        decompose(const QVector<QVector<double>>& A);

    /** @brief 最小二乘求解 Ax≈b @param A 系数矩阵 @param b 右侧向量 @return 解向量 */
    QVector<double> solveLeastSquares(
        const QVector<QVector<double>>& A,
        const QVector<double>& b);

    /** @brief 行列式(方阵) @param A 方阵 @return 行列式 */
    double determinant(const QVector<QVector<double>>& A);

    /** @brief 矩阵秩 @param A 矩阵 @param tolerance 容差 @return 秩 */
    int rank(const QVector<QVector<double>>& A,
             double tolerance = 1e-10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rows, int cols);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // QRDECOMPOSITION_H
