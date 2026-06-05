#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 稀疏Cholesky分解求解器
 *
 * 针对稀疏对称正定矩阵进行Cholesky分解,利用填充缩减排序
 * 优化计算效率,适用于大规模有限元分析与结构力学求解。
 */
class SparseCholesky4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit SparseCholesky4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);

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
};
