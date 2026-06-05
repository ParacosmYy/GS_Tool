#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏矩阵LU分解求解器
 *
 * 对稀疏矩阵进行LU分解，用于高效求解大规模稀疏线性方程组，
 * 支持动态添加非零元素。
 */
class SparseLU4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalSolves = 0;         ///< 已完成求解次数
        int totalNonZeros = 0;       ///< 非零元素数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SparseLU4(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int n);
    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);
    /** @brief 求解线性方程组 Ax=b */
    QVector<double> solve(const QVector<double>& rhs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成，返回迭代次数和残差 */
    void solveCompleted(int iterations, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<int, int>> m_entries;
};
