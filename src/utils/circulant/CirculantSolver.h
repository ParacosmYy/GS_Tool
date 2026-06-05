/**
 * @file CirculantSolver.h
 * @brief 循环矩阵求解器 — FFT加速
 *
 * 功能: 利用循环矩阵对角化性质，通过FFT求解循环线性系统，
 *       复杂度O(n log n)而非一般O(n^3)。
 *
 * 协作: ButterflyOperation(FFT蝶形) / TridiagonalSolver(三对角)
 */
#ifndef CIRCULANTSOLVER_H
#define CIRCULANTSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief 循环矩阵求解器
 */
class CirculantSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit CirculantSolver(QObject* parent = nullptr);

    /**
     * @brief 求解循环系统 Cx = b
     * @param firstColumn 循环矩阵第一列
     * @param rhs 右端向量
     * @return 解向量
     */
    QVector<double> solve(const QVector<double>& firstColumn,
                           const QVector<double>& rhs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 */
    void solveCompleted(int size);

private:
    /** @brief 内部DFT(不依赖外部FFT) */
    QVector<double> dftDiag(const QVector<double>& input) const;

    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // CIRCULANTSOLVER_H
