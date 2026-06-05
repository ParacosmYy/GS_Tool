#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏Cholesky分解求解器
 *
 * 对稀疏对称正定矩阵进行Cholesky分解，
 * 利用稀疏性减少计算量和内存占用。
 */
class SparseCholesky5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit SparseCholesky5(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 求解线性方程组 Ax=b */
    QVector<double> solve(const QVector<double>& rhs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回迭代次数和残差 */
    void solveCompleted(int iterations, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
