/**
 * @file ThomasAlgorithm4.h
 * @brief 周期三对角Thomas算法(Sherman-Morrison辅助向量+环绕修正) — Thomas Algorithm for Periodic Tridiagonal Systems with Sherman-Morrison Auxiliary Vector and Wrap Correction
 *
 * 功能: 实现周期三对角线性方程组的快速求解，采用Sherman-Morrison公式
 *       将周期系统转化为标准三对角系统，辅以环绕修正恢复周期性。
 *
 * 协作: LUdecompose5(LU分解) / GaussSeidel6(Gauss-Seidel) / SparseSolver7(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期三对角Thomas算法(Sherman-Morrison辅助向量)
 */
class ThomasAlgorithm4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        bool isPeriodic = false;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm4(QObject *parent = nullptr);
    ~ThomasAlgorithm4() override;

    /** @brief Solve standard tridiagonal system: a[i]*x[i-1] + b[i]*x[i] + c[i]*x[i+1] = d[i] */
    QVector<double> solve(const QVector<double>& a, const QVector<double>& b,
                          const QVector<double>& c, const QVector<double>& d);

    /** @brief Solve periodic tridiagonal system with Sherman-Morrison */
    QVector<double> solvePeriodic(const QVector<double>& a, const QVector<double>& b,
                                   const QVector<double>& c, const QVector<double>& d);

    /** @brief Compute residual norm ||Ax - d|| */
    double residualNorm(const QVector<double>& a, const QVector<double>& b,
                         const QVector<double>& c, const QVector<double>& d,
                         const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, bool periodic, double residual, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward sweep of standard Thomas algorithm */
    void forwardSweep(QVector<double>& bMod, QVector<double>& dMod,
                      const QVector<double>& a, const QVector<double>& b,
                      const QVector<double>& c, const QVector<double>& d) const;

    /** @brief Back substitution of standard Thomas algorithm */
    QVector<double> backSubstitute(const QVector<double>& bMod,
                                    const QVector<double>& c,
                                    const QVector<double>& dMod) const;
};
