/**
 * @file ThomasAlgorithm7.h
 * @brief Thomas算法(Sherman-Morrison修正的周期三对角系统求解) — Thomas Algorithm with Sherman-Morrison Modification for Periodic Tridiagonal System Solving
 *
 * 功能: 实现Thomas算法(Thomas algorithm)，采用Sherman-Morrison修正(Sherman-Morrison modification)
 *       实现周期三对角系统求解(periodic tridiagonal system solving)。
 *
 * 协作: GaussElimination6(高斯消元) / LUDecomposition8(LU分解) / IterativeSolver5(迭代求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Thomas算法(Sherman-Morrison修正的周期三对角系统求解)
 */
class ThomasAlgorithm7 : public QObject {
    Q_OBJECT

public:
    /** @brief Tridiagonal system definition */
    struct TridiagonalSystem {
        QVector<double> lower;      // Sub-diagonal a[1..n-1]
        QVector<double> main;       // Main diagonal b[0..n-1]
        QVector<double> upper;      // Super-diagonal c[0..n-2]
        QVector<double> rhs;        // Right-hand side d[0..n-1]
    };

    /** @brief Periodic system with corner elements */
    struct PeriodicSystem {
        QVector<double> lower;
        QVector<double> main;
        QVector<double> upper;
        QVector<double> rhs;
        double topLeft = 0.0;       // A[0][n-1] element
        double bottomRight = 0.0;   // A[n-1][0] element
    };

    /** @brief Solution result */
    struct SolveResult {
        QVector<double> solution;
        double residual = 0.0;
        int size = 0;
        bool success = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm7(QObject *parent = nullptr);
    ~ThomasAlgorithm7() override;

    /** @brief Solve standard tridiagonal system */
    SolveResult solve(const TridiagonalSystem& system);

    /** @brief Solve periodic tridiagonal system via Sherman-Morrison */
    SolveResult solvePeriodic(const PeriodicSystem& system);

    /** @brief Compute residual ||Ax - b|| */
    double computeResidual(const TridiagonalSystem& system,
                            const QVector<double>& x) const;

    /** @brief Build tridiagonal system from dense matrix (extract bands) */
    TridiagonalSystem extractBands(const QVector<QVector<double>>& matrix,
                                    const QVector<double>& rhs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward elimination phase */
    void forwardElimination(QVector<double>& a, QVector<double>& b,
                             QVector<double>& c, QVector<double>& d) const;

    /** @brief Back substitution phase */
    QVector<double> backSubstitution(const QVector<double>& b,
                                      const QVector<double>& c,
                                      const QVector<double>& d) const;
};
