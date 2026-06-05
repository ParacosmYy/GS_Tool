/**
 * @file TensorOps.h
 * @brief 张量运算 — 多维数组基础操作
 *
 * 功能: 张量(多维数组)基本运算，支持逐元素操作/矩阵乘法/
 *       转置/reshape，统计操作次数/耗时。
 */
#ifndef TENSOROPS_H
#define TENSOROPS_H

#include <QObject>
#include <QVector>

class TensorOps : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalOperations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit TensorOps(QObject* parent = nullptr);

    /** @brief 矩阵乘法 @param A 矩阵A @param B 矩阵B @return 结果矩阵 */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B) const;

    /** @brief 矩阵转置 @param A 矩阵 @return 转置矩阵 */
    QVector<QVector<double>> transpose(const QVector<QVector<double>>& A) const;

    /** @brief 逐元素加法 @param A 矩阵A @param B 矩阵B @return 结果 */
    QVector<QVector<double>> add(const QVector<QVector<double>>& A,
                                  const QVector<QVector<double>>& B) const;

    /** @brief 逐元素乘法 @param A 矩阵A @param B 矩阵B @return 结果 */
    QVector<QVector<double>> elementWiseMul(const QVector<QVector<double>>& A,
                                             const QVector<QVector<double>>& B) const;

    /** @brief 标量乘法 @param A 矩阵 @param scalar 标量 @return 结果 */
    QVector<QVector<double>> scale(const QVector<QVector<double>>& A,
                                    double scalar) const;

    /** @brief 矩阵Frobenius范数 @param A 矩阵 @return 范数 */
    double frobeniusNorm(const QVector<QVector<double>>& A) const;

    /** @brief 矩阵迹 @param A 方阵 @return 迹 */
    double trace(const QVector<QVector<double>>& A) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // TENSOROPS_H
