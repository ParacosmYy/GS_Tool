#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏LU分解工具类
 *
 * 提供稀疏矩阵的LU分解与求解功能，适用于大型稀疏
 * 线性方程组的高效求解。
 */
class SparseLU3 : public QObject {
    Q_OBJECT
public:
    /// 求解统计信息
    struct Stats {
        int totalSolves = 0;        ///< 总求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SparseLU3(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 */
    void setDimension(int dim);

    /** @brief 添加稀疏矩阵非零元素 */
    void addEntry(int row, int col, double value);

    /** @brief 求解线性方程组 Ax=b */
    QVector<double> solve(const QVector<double>& rhs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回维度和残差 */
    void solveCompleted(int dimension, double residual);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<QPair<int, int>, double>> m_entries;
};
