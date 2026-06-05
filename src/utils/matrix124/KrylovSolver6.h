#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief KrylovSolver6 - Krylov子空间求解器第6代实现
 *
 * 提供GMRES、BiCGSTAB等Krylov子空间迭代方法，
 * 适用于大型稀疏线性方程组求解，支持预处理。
 */
class KrylovSolver6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit KrylovSolver6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief GMRES算法求解线性方程组 Ax = b
     * @param matrix 系数矩阵
     * @param rhs 右端向量
     * @param maxIterations 最大迭代次数
     * @param tolerance 收敛容差
     * @return 解向量
     */
    QVector<double> gmres(const QVector<QVector<double>>& matrix,
                          const QVector<double>& rhs,
                          int maxIterations = 100, double tolerance = 1e-8);

    /**
     * @brief BiCGSTAB算法求解线性方程组
     * @param matrix 系数矩阵
     * @param rhs 右端向量
     * @param maxIterations 最大迭代次数
     * @param tolerance 收敛容差
     * @return 解向量
     */
    QVector<double> bicgstab(const QVector<QVector<double>>& matrix,
                             const QVector<double>& rhs,
                             int maxIterations = 100, double tolerance = 1e-8);

    /**
     * @brief 设置预处理方法
     * @param precondType 预处理类型 (None/Jacobi/ILU/SGS)
     */
    void setPreconditioner(const QString& precondType);

    /**
     * @brief 获取最近一次求解的残差历史
     * @return 残差值序列
     */
    QVector<double> residualHistory() const;

signals:
    void solveCompleted(int iterationCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
