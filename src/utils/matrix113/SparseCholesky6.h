#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏Cholesky分解求解器实现
 *
 * 对稀疏对称正定矩阵执行Cholesky分解(L*L^T)，利用填充减少排序
 * 保持稀疏性，适用于大规模有限元分析和结构力学方程求解。
 */
class SparseCholesky6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecomposed = 0; double avgProcessingTimeMs = 0.0; };

    explicit SparseCholesky6(QObject* parent = nullptr);

    /** @brief 设置矩阵的维度并预分配稀疏存储结构 */
    void setDimension(int n);

    /** @brief 添加非零元素到稀疏矩阵，格式为(行,列,值) */
    void addEntry(int row, int col, double value);

    /** @brief 执行稀疏Cholesky分解，返回下三角因子L的非零元 */
    QVector<QPair<QPair<int, int>, double>> decompose();

    /** @brief 利用已有分解结果求解线性方程组Ax=b */
    QVector<double> solve(const QVector<double>& b);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成信号，返回矩阵维度和非零元数 */
    void decompositionCompleted(int dimension, int nonZeros);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
};
