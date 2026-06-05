/**
 * @file TensorDecomposition.h
 * @brief 张量CP分解 — 简化版CANDECOMP/PARAFAC
 *
 * 功能: 对三维张量执行CP分解，将其分解为秩为R的因子矩阵。
 *       使用交替最小二乘(ALS)算法迭代求解因子矩阵。
 *
 * 协作: TensorOps(基础张量运算) / PcaTransform(矩阵降维)
 */
#ifndef TENSORDECOMPOSITION_H
#define TENSORDECOMPOSITION_H

#include <QObject>
#include <QVector>

/**
 * @brief 张量CP分解引擎
 *
 * 将三维张量 X 分解为 A * B * C 三组因子矩阵，
 * 其中 rank R 控制分解精度。ALS迭代最小化重构误差。
 */
class TensorDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecomposed = 0;       ///< 累计分解次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit TensorDecomposition(QObject* parent = nullptr);

    /**
     * @brief 设置CP分解的秩
     * @param r 目标秩(必须 > 0)
     */
    void setRank(int r);

    /**
     * @brief 执行CP分解(ALS)
     * @param tensor 三维张量 [I][J][K]
     * @param maxIter 最大ALS迭代次数(默认100)
     * @return 因子矩阵列表 {A(I×R), B(J×R), C(K×R)}
     */
    QVector<QVector<double>> decompose(
        const QVector<QVector<QVector<double>>>& tensor,
        int maxIter = 100);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param rank 分解秩 */
    void decompositionCompleted(int rank);

private:
    /**
     * @brief Khatri-Rao积 (B ⊙ C)
     * @param B 因子矩阵B (J×R)
     * @param C 因子矩阵C (K×R)
     * @return Khatri-Rao积矩阵 (JK×R)
     */
    static QVector<QVector<double>> khatriRao(
        const QVector<QVector<double>>& B,
        const QVector<QVector<double>>& C);

    /**
     * @brief 矩阵伪逆 (Moore-Penrose)
     * @param A 输入矩阵
     * @return 伪逆矩阵
     */
    static QVector<QVector<double>> pseudoInverse(
        const QVector<QVector<double>>& A);

    /**
     * @brief 矩阵乘法 C = A^T * B
     * @param A 矩阵A
     * @param B 矩阵B
     * @return 乘积矩阵
     */
    static QVector<QVector<double>> matMulATB(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    int m_rank = 2;             ///< CP分解秩
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs = 0.0;   ///< 累计耗时(ms)
};

#endif // TENSORDECOMPOSITION_H
