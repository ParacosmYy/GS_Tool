/**
 * @file PowerIteration.h
 * @brief 幂迭代法 — 特征值/特征向量计算
 *
 * 功能: 使用幂迭代法求矩阵最大特征值及对应特征向量。
 *       支持逆迭代求最小特征值和移位迭代求任意特征值。
 *
 * 协作: QrDecomposition(QR迭代) / PcaAnalyzer(主成分)
 */
#ifndef POWERITERATION_H
#define POWERITERATION_H

#include <QObject>
#include <QVector>

/**
 * @brief 幂迭代法特征值计算
 */
class PowerIteration : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIterations = 0;     ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 特征结果 */
    struct EigenResult {
        double eigenvalue = 0.0;         ///< 特征值
        QVector<double> eigenvector;     ///< 特征向量(归一化)
        int iterations = 0;              ///< 迭代次数
        bool converged = false;          ///< 是否收敛
    };

    explicit PowerIteration(QObject* parent = nullptr);

    /** @brief 幂迭代求最大特征值
     *  @param matrix 方阵
     *  @param maxIter 最大迭代次数
     *  @param tol 收敛容限
     *  @return 特征结果 */
    EigenResult largestEigenvalue(
        const QVector<QVector<double>>& matrix,
        int maxIter = 1000, double tol = 1e-10);

    /** @brief 逆迭代求最小特征值
     *  @param matrix 方阵
     *  @param maxIter 最大迭代次数
     *  @param tol 收敛容限
     *  @return 特征结果 */
    EigenResult smallestEigenvalue(
        const QVector<QVector<double>>& matrix,
        int maxIter = 1000, double tol = 1e-10);

    /** @brief 移位逆迭代求指定特征值附近的特征对
     *  @param matrix 方阵
     *  @param shift 移位值
     *  @param maxIter 最大迭代次数
     *  @param tol 收敛容限
     *  @return 特征结果 */
    EigenResult shiftInvert(
        const QVector<QVector<double>>& matrix,
        double shift,
        int maxIter = 1000, double tol = 1e-10);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 迭代完成 @param iter 迭代号 @param eigenvalue 当前特征值 */
    void iterationCompleted(int iter, double eigenvalue);

    /** @brief 计算完成 @param eigenvalue 最终特征值 @param converged 是否收敛 */
    void computationCompleted(double eigenvalue, bool converged);

private:
    /** @brief 矩阵向量乘 */
    QVector<double> matVecMul(const QVector<QVector<double>>& A,
                              const QVector<double>& v) const;

    /** @brief 向量范数 */
    double vecNorm(const QVector<double>& v) const;

    /** @brief 向量归一化 */
    QVector<double> normalize(const QVector<double>& v) const;

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // POWERITERATION_H
