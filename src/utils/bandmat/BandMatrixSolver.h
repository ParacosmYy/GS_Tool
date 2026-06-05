/**
 * @file BandMatrixSolver.h
 * @brief 带状线性方程组求解 — LU 分解
 *
 * 功能: 对带宽为 m 的带状矩阵执行带状 LU 分解(L,U 仅存带内元素)，
 *       然后前代/回代求解。时间复杂度 O(n·m²)。
 *
 * 协作: TridiagonalSolver(三对角特例) / LUDecomposition(一般稠密)
 */
#ifndef BANDMATRIXSOLVER_H
#define BANDMATRIXSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief 带状线性方程组求解器
 */
class BandMatrixSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit BandMatrixSolver(QObject* parent = nullptr);

    /**
     * @brief 求解带状线性方程组
     * @param bandMatrix 带状存储矩阵(n×(2*bandwidth+1))
     * @param rhs 右端向量(n)
     * @param bandwidth 半带宽(对角线一侧的非零对角线条数)
     * @return 解向量(n)
     */
    QVector<double> solve(QVector<QVector<double>> bandMatrix,
                          QVector<double> rhs, int bandwidth);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 方程组规模 */
    void solveCompleted(int size);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // BANDMATRIXSOLVER_H
