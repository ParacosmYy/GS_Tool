/**
 * @file SylvesterSolver.h
 * @brief Sylvester方程求解器 — AX+XB=C
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Sylvester矩阵方程求解器
 * 支持连续和离散Sylvester/Lyapunov方程
 */
class SylvesterSolver : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSolves = 0;
        int totalSchurDecompositions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SylvesterSolver(QObject* parent = nullptr);

    /** @brief 求解连续Sylvester方程 AX + XB = C
     *  @param A m×m矩阵 @param B n×n矩阵 @param C m×n矩阵
     *  @param m A的阶数 @param n B的阶数 @return 解矩阵X(m×n,行优先) */
    QVector<double> solveSylvester(const QVector<double>& A,
                                    const QVector<double>& B,
                                    const QVector<double>& C,
                                    int m, int n);

    /** @brief 求解连续Lyapunov方程 AX + XA^T = C */
    QVector<double> solveLyapunov(const QVector<double>& A,
                                  const QVector<double>& C, int n);

    /** @brief 求解离散Lyapunov方程 AXA^T - X = C */
    QVector<double> solveDiscreteLyapunov(const QVector<double>& A,
                                           const QVector<double>& C, int n);

    /** @brief 检查Sylvester方程可解性(Schur谱无交集) */
    bool isSolvable(const QVector<double>& A, const QVector<double>& B,
                    int m, int n) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int m, int n);

private:
    /** @brief Schur分解(上三角化) */
    void schurDecompose(QVector<double>& mat, int n,
                        QVector<double>& Q) const;

    /** @brief 三角Sylvester求解 */
    QVector<double> triangularSolve(const QVector<double>& T1,
                                     const QVector<double>& T2,
                                     const QVector<double>& F,
                                     int m, int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
