/**
 * @file CholeskySolver.h
 * @brief 稠密Cholesky分解(前向/后向代入+条件数估计) — Dense Cholesky Factorization with Forward/Backward Substitution and Condition Estimation
 *
 * 功能: 实现稠密对称正定矩阵的Cholesky分解(A=LL^T)，支持前向/后向代入
 *       求解线性方程组，以及基于Cholesky因子的条件数估计。
 *
 * 协作: GaussElimination(高斯消元) / LUDecomposition(LU分解) / QRSolver(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解线性求解器
 */
class CholeskySolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFactorizations = 0; ///< 累计分解次数
        quint64 totalSolves = 0;         ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        double lastConditionEst = 0.0;   ///< 最近一次条件数估计
        int lastMatrixSize = 0;          ///< 最近一次矩阵大小
    };

    explicit CholeskySolver(QObject* parent = nullptr);
    ~CholeskySolver() override;

    /**
     * @brief 对对称正定矩阵进行Cholesky分解
     * @param A 对称正定矩阵(仅使用下三角)
     * @return 是否成功(矩阵不正定时返回false)
     */
    bool factorize(const QVector<QVector<double>>& A);

    /**
     * @brief 求解Ax=b(需先调用factorize)
     * @param b 右端向量
     * @return 解向量x
     */
    QVector<double> solve(const QVector<double>& b) const;

    /**
     * @brief 估计矩阵条件数(需先调用factorize)
     * @return 条件数估计值(kappa = ||A|| * ||A^-1||)
     */
    double estimateCondition() const;

    /** @brief 获取下三角因子L */
    QVector<QVector<double>> getL() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵大小 */
    void factorizationCompleted(int n);
    /** @brief 求解完成 @param n 向量大小 */
    void solveCompleted(int n);

private:
    /** @brief 前向代入求解 Ly = b */
    QVector<double> forwardSub(const QVector<double>& b) const;

    /** @brief 后向代入求解 L^T x = y */
    QVector<double> backwardSub(const QVector<double>& y) const;

    int m_n = 0;
    bool m_factored = false;
    QVector<QVector<double>> m_L;

    Stats m_stats;
    double m_timeSum = 0.0;
};
