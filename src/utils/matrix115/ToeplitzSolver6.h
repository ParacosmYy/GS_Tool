#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Toeplitz矩阵求解器实现
 *
 * 利用Levinson-Durbin递归算法高效求解Toeplitz线性方程组，
 * 时间复杂度O(n^2)，适用于自回归模型参数估计和维纳滤波器设计。
 */
class ToeplitzSolver6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalSolved = 0; double avgProcessingTimeMs = 0.0; };

    explicit ToeplitzSolver6(QObject* parent = nullptr);

    /** @brief 设置Toeplitz矩阵的第一行元素，自动对称生成完整矩阵 */
    void setFirstRow(const QVector<double>& row);

    /** @brief 求解Tx=b方程组，返回解向量 */
    QVector<double> solve(const QVector<double>& b);

    /** @brief 计算Toeplitz矩阵的行列式 */
    double determinant() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成信号，返回矩阵维度 */
    void solveCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_firstRow;
};
