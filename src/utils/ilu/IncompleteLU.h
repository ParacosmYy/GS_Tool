/**
 * @file IncompleteLU.h
 * @brief 不完全LU分解(ILU(0)) — 预条件子
 *
 * 功能: ILU(0)不完全LU分解，保持原始稀疏结构，
 *       用作迭代求解器的预条件子。
 *
 * 协作: ConjugateGradient(CG) / SparseDirectSolver(直接法)
 */
#ifndef INCOMPLETELU_H
#define INCOMPLETELU_H

#include <QObject>
#include <QVector>

/**
 * @brief 不完全LU分解预条件子
 */
class IncompleteLU : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalFactorizations = 0;    ///< 累计分解次数
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit IncompleteLU(QObject* parent = nullptr);

    /**
     * @brief ILU(0)分解
     * @param A 输入矩阵
     * @return 成功与否
     */
    bool factorize(const QVector<QVector<double>>& A);

    /**
     * @brief 求解 LUx = b
     * @param rhs 右端向量
     * @return 解向量
     */
    QVector<double> solve(const QVector<double>& rhs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param size 矩阵维度 */
    void factorizationCompleted(int size);

private:
    QVector<QVector<double>> m_LU;  ///< LU组合矩阵
    Stats m_stats;
    double m_timeSum = 0.0;
};

#endif // INCOMPLETELU_H
