/**
 * @file LanczosEigen.h
 * @brief Lanczos算法 — 对称稀疏矩阵特征值求解
 *
 * 功能: 使用Lanczos算法将对称稀疏矩阵三对角化，
 *       然后求解三对角矩阵的特征值来获得原矩阵的近似特征值。
 *       仅需矩阵-向量乘积回调，无需显式存储矩阵。
 *       支持完全重正交化以抵消浮点误差导致的正交性丢失。
 *
 * 协作: DavidsonEigen(非对称扩展) / PowerMethodGeneralized(广义特征值)
 */
#ifndef LANCZOSEIGEN_H
#define LANCZOSEIGEN_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Lanczos特征值求解器
 */
class LanczosEigen : public QObject {
    Q_OBJECT

public:
    /** @brief 矩阵-向量乘积回调类型 */
    using MatVecFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit LanczosEigen(QObject* parent = nullptr);

    /** @brief 求解最大特征值
     *  @param matvec 矩阵-向量乘积回调 A*x
     *  @param n 矩阵维度
     *  @param numEigen 欲求特征值个数
     *  @param maxIter 最大Lanczos迭代次数
     *  @return 特征值数组(降序) */
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

    /** @brief 对三对角矩阵求特征值(隐式QR) */
    static QVector<double> tridiagEigenvalues(
        const QVector<double>& alpha,
        const QVector<double>& beta,
        int numEigen);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // LANCZOSEIGEN_H
