/**
 * @file SparseDirectSolver.h
 * @brief 稀疏LU直接求解器 — 带部分主元
 *
 * 功能: 对稀疏矩阵进行LU分解并求解线性系统，
 *       支持COO格式输入。
 *
 * 协作: IncompleteLU(ILU预处理) / ConjugateGradient(CG迭代)
 */
#ifndef SPAREDIRECTSOLVER_H
#define SPAREDIRECTSOLVER_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏LU直接求解器
 */
class SparseDirectSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit SparseDirectSolver(QObject* parent = nullptr);

    /**
     * @brief 求解稀疏线性系统
     * @param entries 非零元素坐标(row, col)列表
     * @param values 非零元素值列表
     * @param rhs 右端向量
     * @param n 矩阵维度
     * @return 解向量
     */
    QVector<double> solve(const QVector<QPair<int, int>>& entries,
                           const QVector<double>& values,
                           const QVector<double>& rhs, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 */
    void solveCompleted(int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

#endif // SPAREDIRECTSOLVER_H
