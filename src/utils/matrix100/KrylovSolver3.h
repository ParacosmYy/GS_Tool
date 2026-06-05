#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Krylov子空间迭代求解器
 *
 * 基于GMRES/CG等Krylov子空间方法求解大规模稀疏线性方程组,
 * 适用于计算流体力学、电磁场仿真等超大规模科学计算场景。
 */
class KrylovSolver3 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit KrylovSolver3(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 设置最大迭代次数 */
    void setMaxIter(int maxIter);

    /** @brief 求解线性方程组 Ax=b */
    void solve(const QVector<double>& rhs);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号,返回迭代次数与残差 */
    void solveCompleted(int iterations, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    int m_maxIter = 1000;
};
