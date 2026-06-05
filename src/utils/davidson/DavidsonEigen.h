/**
 * @file DavidsonEigen.h
 * @brief Davidson方法 — 大型稀疏矩阵特征值求解
 *
 * 功能: 使用Davidson方法求解大型稀疏对称矩阵的若干最小特征值。
 *       仅需矩阵-向量乘积回调，无需显式存储矩阵。
 *       适用于量子化学和大规模工程问题中的稀疏特征值问题。
 *
 * 协作: LanczosEigen(对称三对角化) / PowerMethodGeneralized(广义特征值)
 */
#ifndef DAVIDSONEIGEN_H
#define DAVIDSONEIGEN_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Davidson特征值求解器
 */
class DavidsonEigen : public QObject {
    Q_OBJECT

public:
    /** @brief 矩阵-向量乘积回调类型 */
    using MatVecFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit DavidsonEigen(QObject* parent = nullptr);

    /** @brief 求解最小特征值
     *  @param matvec 矩阵-向量乘积回调 A*x
     *  @param n 矩阵维度
     *  @param numEigen 欲求特征值个数
     *  @param maxIter 最大迭代次数
     *  @return 特征值数组(升序) */
    QVector<double> solve(const MatVecFunc& matvec,
                          int n,
                          int numEigen = 1,
                          int maxIter = 200);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param eigenvalueCount 特征值数 @param iterations 迭代次数 */
    void solveCompleted(int eigenvalueCount, int iterations);

private:
    /** @brief 向量点积 */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief 向量范数 */
    static double vecNorm(const QVector<double>& v);

    /** @brief 正交化向量对已有基 */
    static void orthogonalizeAgainst(QVector<double>& v,
                                     const QVector<QVector<double>>& basis);

    /** @brief 求解小规模特征值问题(使用QR迭代) */
    static QVector<double> smallEigenSolve(const QVector<QVector<double>>& mat,
                                           int numEigen,
                                           QVector<QVector<double>>& eigvecs);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // DAVIDSONEIGEN_H
