/**
 * @file PowerMethodGeneralized.h
 * @brief 广义幂法 — 广义特征值问题 Ax = λBx
 *
 * 功能: 使用幂法迭代求解广义特征值问题 Ax = λBx 的最大特征值
 *       及对应特征向量。通过Cholesky分解预处理确保收敛性。
 *       适用于对称正定B矩阵的广义特征值问题。
 *
 * 协作: LanczosEigen(稀疏特征值) / DavidsonEigen(大规模稀疏)
 */
#ifndef POWERMETHODGENERALIZED_H
#define POWERMETHODGENERALIZED_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值幂法求解器
 */
class PowerMethodGeneralized : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit PowerMethodGeneralized(QObject* parent = nullptr);

    /** @brief 求解最大广义特征值
     *  @param A 系数矩阵
     *  @param B 质量矩阵(对称正定)
     *  @param maxIter 最大迭代次数
     *  @param tol 收敛容限
     *  @return 特征值和特征向量 */
    QPair<double, QVector<double>> solve(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        int maxIter = 200,
        double tol = 1e-10);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param eigenvalue 特征值 @param iterations 迭代次数 */
    void solveCompleted(double eigenvalue, int iterations);

private:
    /** @brief 矩阵-向量乘积 */
    static QVector<double> matVecMul(const QVector<QVector<double>>& mat,
                                     const QVector<double>& v);

    /** @brief 向量点积 */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief 向量范数 */
    static double vecNorm(const QVector<double>& v);

    /** @brief 简单Cholesky求解 Ly=b, L^T x=y */
    static QVector<double> choleskySolve(
        const QVector<QVector<double>>& mat,
        const QVector<double>& rhs);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // POWERMETHODGENERALIZED_H
