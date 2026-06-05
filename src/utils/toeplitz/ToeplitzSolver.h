/**
 * @file ToeplitzSolver.h
 * @brief Toeplitz线性系统求解器 — Levinson-Durbin算法
 *
 * 功能: 使用Levinson-Durbin递归算法求解Toeplitz线性方程组 Tx=b。
 *       时间复杂度O(n^2)，远优于一般矩阵求解的O(n^3)。
 *       广泛应用于线性预测编码(LPC)、自回归模型和信号处理。
 *
 * 协作: YuleWalker(Yule-Walker方程) / AutoCorrelator(自相关)
 */
#ifndef TOEPLITZSOLVER_H
#define TOEPLITZSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Toeplitz系统求解器
 */
class ToeplitzSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit ToeplitzSolver(QObject* parent = nullptr);

    /** @brief 求解Toeplitz线性系统 Tx = b
     *  @param firstColumn Toeplitz矩阵第一列(定义整个矩阵)
     *  @param rhs 右端向量b
     *  @return 解向量x */
    QVector<double> solve(const QVector<double>& firstColumn,
                          const QVector<double>& rhs);

    /** @brief Levinson-Durbin求解Yule-Walker方程
     *  @param autocorr 自相关序列 r[0..n-1]
     *  @return 反射系数和预测误差功率 */
    QPair<QVector<double>, double> levinsonDurbin(
        const QVector<double>& autocorr);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 系统规模 */
    void solveCompleted(int size);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // TOEPLITZSOLVER_H
