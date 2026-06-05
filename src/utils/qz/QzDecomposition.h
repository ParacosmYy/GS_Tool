/**
 * @file QzDecomposition.h
 * @brief QZ分解 — 广义特征值问题 Ax = λBx
 *
 * 功能: 对矩阵对(A, B)进行QZ分解，计算广义特征值，
 *       基于QZ步迭代(Givens旋转)，统计分解次数与平均耗时。
 *
 * 协作: QrDecomposition(QR分解) / SvdSolver(奇异值)
 */
#ifndef QZDECOMPOSITION_H
#define QZDECOMPOSITION_H

#include <QObject>
#include <QVector>

class QzDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit QzDecomposition(QObject* parent = nullptr);

    /**
     * @brief 对矩阵对(A, B)进行QZ分解，求广义特征值
     * @param matA 矩阵A(n×n)
     * @param matB 矩阵B(n×n)
     * @param maxIter 最大迭代次数(默认300)
     * @return 广义特征值数组(长度n)
     */
    QVector<double> decompose(const QVector<QVector<double>>& matA,
                              const QVector<QVector<double>>& matB,
                              int maxIter = 300);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param sz 矩阵尺寸 */
    void decompositionCompleted(int sz);

private:
    Stats  m_stats;
    double m_timeSum;
};

#endif // QZDECOMPOSITION_H
