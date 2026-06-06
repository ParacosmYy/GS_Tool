/**
 * @file BandedSolver.h
 * @brief 带状矩阵求解器(Thomas推广+任意带宽) — Banded Matrix Solver with Thomas Generalization for Arbitrary Bandwidth
 *
 * 功能: 实现带状矩阵线性方程组求解，支持Thomas算法推广至任意带宽、
 *       LU分解带状存储和前代/回代求解。
 *
 * 协作: LUDecomposer3(LU分解) / CholeskySolver4(Cholesky) / GaussSolver5(高斯消元)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 带状矩阵求解器
 */
class BandedSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;           ///< 累计求解次数
        int lastSize = 0;                  ///< 最近矩阵尺寸
        int lastBandwidth = 0;             ///< 最近带宽
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit BandedSolver(QObject *parent = nullptr);
    ~BandedSolver() override;

    /**
     * @brief 求解带状线性方程组 Ax = b
     * @param A 带状矩阵(紧凑存储：A[row]含从col=row-kl到col=row+ku的元素)
     * @param b 右端向量
     * @param kl 下带宽
     * @param ku 上带宽
     * @return 解向量x
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b,
                          int kl, int ku);

    /**
     * @brief Thomas算法(三对角专用)
     * @param lower 下对角线(n-1)
     * @param main 主对角线(n)
     * @param upper 上对角线(n-1)
     * @param rhs 右端向量(n)
     * @return 解向量(n)
     */
    static QVector<double> thomas(const QVector<double>& lower,
                                   const QVector<double>& main,
                                   const QVector<double>& upper,
                                   const QVector<double>& rhs);

    /** @brief 将带状矩阵转为紧凑存储格式 */
    static QVector<QVector<double>> packBanded(
        const QVector<QVector<double>>& full, int kl, int ku);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int bandwidth);

private:
    /** @brief 带状LU分解(无主元) */
    bool luDecompose(QVector<QVector<double>>& A, int kl, int ku) const;

    /** @brief 前代 */
    QVector<double> forwardSub(const QVector<QVector<double>>& A,
                               const QVector<double>& b, int kl) const;

    /** @brief 回代 */
    QVector<double> backwardSub(const QVector<QVector<double>>& A,
                                const QVector<double>& y, int kl, int ku) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
