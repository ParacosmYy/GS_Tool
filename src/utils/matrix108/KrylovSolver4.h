#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Krylov子空间迭代求解器
 *
 * 基于GMRES/BiCGSTAB等Krylov子空间方法求解大型稀疏线性系统，
 * 适用于非对称矩阵的高效迭代求解。
 */
class KrylovSolver4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit KrylovSolver4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 设置最大迭代次数 */
    void setMaxIter(int maxIter);

    /** @brief 求解线性方程组 Ax=b */
    QVector<double> solve(const QVector<double>& rhs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solveCompleted(int iterations, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    int m_maxIter = 1000;
};
