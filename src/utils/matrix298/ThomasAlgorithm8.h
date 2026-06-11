/**
 * @file ThomasAlgorithm8.h
 * @brief 追赶法(部分选主元与边界条件处理实现鲁棒三对角线性方程组求解) — Thomas Algorithm with Partial Pivoting and Boundary Condition Handling for Robust Tridiagonal System Solution
 *
 * 功能: 实现追赶法(Thomas algorithm)，采用部分选主元(partial pivoting)
 *       与边界条件处理(boundary condition handling)实现鲁棒三对角线性方程组求解(robust tridiagonal system solution)。
 *
 * 协作: LUDecomposition9(LU分解) / GaussSeidel7(Gauss-Seidel) / ConjugateGradient6(共轭梯度)
 */
#pragma once

#include <QObject>
#include <QVector>

class ThomasAlgorithm8 : public QObject {
    Q_OBJECT

public:
    /** @brief Boundary condition type */
    enum BoundaryType {
        Dirichlet,      // Fixed value at boundary
        Neumann,        // Fixed derivative at boundary
        Periodic        // Periodic boundary
    };

    /** @brief Solve result */
    struct SolveResult {
        QVector<double> solution;
        double residual = 0.0;
        int systemSize = 0;
        bool converged = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int maxSystemSize = 0;
        double avgResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm8(QObject *parent = nullptr);
    ~ThomasAlgorithm8() override;

    /** @brief Set boundary conditions for left and right ends */
    void setBoundaryConditions(BoundaryType left, double leftValue,
                                BoundaryType right, double rightValue);

    /** @brief Solve tridiagonal system: lower*sub + main*diag + upper*sup = rhs */
    SolveResult solve(const QVector<double>& lower,
                       const QVector<double>& mainDiag,
                       const QVector<double>& upper,
                       const QVector<double>& rhs);

    /** @brief Solve with partial pivoting for robustness */
    SolveResult solveWithPivoting(const QVector<double>& lower,
                                    const QVector<double>& mainDiag,
                                    const QVector<double>& upper,
                                    const QVector<double>& rhs);

    /** @brief Compute residual ||Ax - b|| */
    double computeResidual(const QVector<double>& lower,
                            const QVector<double>& mainDiag,
                            const QVector<double>& upper,
                            const QVector<double>& rhs,
                            const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, double residual, double timeMs);

private:
    BoundaryType m_leftType = Dirichlet;
    double m_leftValue = 0.0;
    BoundaryType m_rightType = Dirichlet;
    double m_rightValue = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_resSum = 0.0;

    /** @brief Apply boundary modifications to system */
    void applyBoundaryConditions(QVector<double>& mainDiag,
                                  QVector<double>& upper,
                                  QVector<double>& lower,
                                  QVector<double>& rhs) const;

    /** @brief Handle periodic boundary via Sherman-Morrison */
    SolveResult solvePeriodic(QVector<double> lower,
                                QVector<double> mainDiag,
                                QVector<double> upper,
                                QVector<double> rhs);
};
