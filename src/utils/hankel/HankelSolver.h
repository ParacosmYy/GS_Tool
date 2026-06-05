/**
 * @file HankelSolver.h
 * @brief Hankel矩阵求解器 — 矩问题
 *
 * 功能: 求解Hankel矩阵线性系统，用于矩量和Padé逼近，
 *       基于LU分解。
 *
 * 协作: PadeApproximant(Pade逼近) / ToeplitzSolver(Toeplitz系统)
 */
#ifndef HANKELSOLVER_H
#define HANKELSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Hankel矩阵求解器
 */
class HankelSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit HankelSolver(QObject* parent = nullptr);

    /**
     * @brief 求解Hankel系统
     * @param moments 第一行/列(矩量序列)
     * @param rhs 右端向量
     * @return 解向量
     */
    QVector<double> solve(const QVector<double>& moments,
                           const QVector<double>& rhs);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 */
    void solveCompleted(int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

#endif // HANKELSOLVER_H
