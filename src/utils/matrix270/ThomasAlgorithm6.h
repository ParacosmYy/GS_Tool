/**
 * @file ThomasAlgorithm6.h
 * @brief Thomas算法(部分主元消去与循环约化扩展鲁棒三对角系统求解) — Thomas Algorithm with Partial Pivoting and Cyclic Reduction Extension for Robust Tridiagonal System Solving
 *
 * 功能: 实现Thomas算法(Thomas algorithm)，采用部分主元消去(partial pivoting)
 *       和循环约化扩展(cyclic reduction extension)实现鲁棒三对角系统求解。
 *
 * 协作: GaussElimination7(高斯消元) / LUDecomposition6(LU分解) / Cholesky5(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Thomas算法(部分主元消去与循环约化扩展鲁棒三对角系统求解)
 */
class ThomasAlgorithm6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        bool pivoted = false;
        int pivotCount = 0;
        bool isCyclic = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm6(QObject *parent = nullptr);
    ~ThomasAlgorithm6() override;

    /** @brief Solve standard tridiagonal system Ax=d */
    QVector<double> solve(const QVector<double>& lower,
                          const QVector<double>& mainDiag,
                          const QVector<double>& upper,
                          const QVector<double>& rhs);

    /** @brief Solve with partial pivoting for robustness */
    QVector<double> solveWithPivoting(QVector<double> lower,
                                      QVector<double> mainDiag,
                                      QVector<double> upper,
                                      QVector<double> rhs);

    /** @brief Solve cyclic tridiagonal system via Sherman-Morrison */
    QVector<double> solveCyclic(const QVector<double>& lower,
                                const QVector<double>& mainDiag,
                                const QVector<double>& upper,
                                const QVector<double>& rhs,
                                double cornerLower, double cornerUpper);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void systemSolved(int size, bool pivoted, int pivots, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard Thomas forward elimination + back substitution */
    QVector<double> thomasSolve(QVector<double> a, QVector<double> b,
                                QVector<double> c, QVector<double> d);
};
