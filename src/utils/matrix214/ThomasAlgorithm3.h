/**
 * @file ThomasAlgorithm3.h
 * @brief 块三对角Thomas算法(Thomas-Block循环约化+Sherman-Morrison耦合) — Thomas Algorithm for Block Tridiagonal Systems with Thomas-Block Cyclic Reduction and Sherman-Morrison Coupling
 *
 * 功能: 实现块三对角Thomas算法，支持Thomas-Block循环约化、
 *       Sherman-Morrison耦合修正和周期性边界条件。
 *
 * 协作: GaussianElimination6(高斯消元) / LUDecomposition5(LU分解) / IterativeSolver4(迭代求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 块三对角Thomas算法(Thomas-Block循环约化+Sherman-Morrison耦合)
 */
class ThomasAlgorithm3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int systemSize = 0;
        int blockSize = 0;
        bool periodic = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm3(QObject *parent = nullptr);
    ~ThomasAlgorithm3() override;

    /** @brief Solve scalar tridiagonal system: a_i x_{i-1} + b_i x_i + c_i x_{i+1} = d_i */
    QVector<double> solveScalar(
        const QVector<double>& lower,
        const QVector<double>& diag,
        const QVector<double>& upper,
        const QVector<double>& rhs) const;

    /** @brief Solve periodic (cyclic) scalar tridiagonal system */
    QVector<double> solveScalarPeriodic(
        const QVector<double>& lower,
        const QVector<double>& diag,
        const QVector<double>& upper,
        const QVector<double>& rhs) const;

    /** @brief Solve block tridiagonal system with block cyclic reduction */
    QVector<QVector<double>> solveBlock(
        const QVector<QVector<QVector<double>>>& lowerBlocks,
        const QVector<QVector<QVector<double>>>& diagBlocks,
        const QVector<QVector<QVector<double>>>& upperBlocks,
        const QVector<QVector<double>>& rhsBlocks) const;

    /** @brief Solve periodic block tridiagonal with Sherman-Morrison coupling */
    QVector<QVector<double>> solveBlockPeriodic(
        const QVector<QVector<QVector<double>>>& lowerBlocks,
        const QVector<QVector<QVector<double>>>& diagBlocks,
        const QVector<QVector<QVector<double>>>& upperBlocks,
        const QVector<QVector<double>>& rhsBlocks) const;

    /** @brief Invert a small matrix via Gauss-Jordan */
    static QVector<QVector<double>> invertMatrix(
        const QVector<QVector<double>>& mat);

    /** @brief Multiply two matrices */
    static QVector<QVector<double>> multiply(
        const QVector<QVector<double>>& a,
        const QVector<QVector<double>>& b);

    /** @brief Subtract two vectors */
    static QVector<double> vecSub(
        const QVector<double>& a, const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int blockSize, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sherman-Morrison rank-1 update solve for periodic case */
    QVector<double> shermanMorrisonSolve(
        const QVector<double>& lower,
        const QVector<double>& diag,
        const QVector<double>& upper,
        const QVector<double>& rhs,
        double gamma, int couplingIdx) const;
};
