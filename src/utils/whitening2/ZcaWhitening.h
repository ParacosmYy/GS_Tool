/**
 * @file ZcaWhitening.h
 * @brief ZCA白化变换 — 零相位分量分析白化
 *
 * 功能: 对数据进行ZCA(Mahalanobis)白化变换，使输出各维度不相关且方差为1，
 *       同时保持与原始数据最大相似度。支持fit/transform/inverse完整流水线。
 *
 * 协作: PcaTransform(PCA降维) / Normalize(标准化) / Covariance(协方差)
 */
#ifndef ZCAWHITENING_H
#define ZCAWHITENING_H

#include <QObject>
#include <QVector>

/**
 * @brief ZCA白化变换器
 *
 * 白化矩阵 W = E * D^(-1/2) * E^T，其中E为特征向量，D为特征值。
 * 变换后 y = W * (x - mean)，各维度不相关且方差归一。
 * ZCA相比PCA白化保持与原始数据最大余弦相似度。
 */
class ZcaWhitening : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTransformed = 0;      ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ZcaWhitening(QObject* parent = nullptr);

    /**
     * @brief 拟合白化模型(计算均值、协方差、白化矩阵)
     * @param data 输入数据矩阵 [n_samples][n_features]
     */
    void fit(const QVector<QVector<double>>& data);

    /**
     * @brief 使用已拟合模型变换数据
     * @param data 输入数据矩阵 [n_samples][n_features]
     * @return 白化后的数据矩阵
     */
    QVector<QVector<double>> transform(
        const QVector<QVector<double>>& data) const;

    /**
     * @brief 逆变换(从白化空间还原)
     * @param whitened 白化后的数据矩阵
     * @return 还原后的数据矩阵
     */
    QVector<QVector<double>> inverse(
        const QVector<QVector<double>>& whitened) const;

    /** @brief 是否已拟合 */
    bool isFitted() const { return m_fitted; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 @param dimensions 特征维度数 */
    void fitted(int dimensions);

private:
    /**
     * @brief 对称矩阵特征分解(Jacobi迭代)
     * @param matrix 对称矩阵
     * @param eigenValues 输出特征值
     * @param eigenVectors 输出特征向量(列存储)
     * @return 是否成功
     */
    static bool eigenDecompose(const QVector<QVector<double>>& matrix,
                               QVector<double>& eigenValues,
                               QVector<QVector<double>>& eigenVectors);

    bool m_fitted = false;                  ///< 是否已拟合
    int m_dimensions = 0;                   ///< 特征维度数
    QVector<double> m_mean;                 ///< 各维度均值
    QVector<QVector<double>> m_whiteningMatrix;  ///< 白化矩阵
    QVector<QVector<double>> m_dewhiteningMatrix; ///< 逆白化矩阵
    mutable Stats m_stats;                  ///< 统计信息
    mutable double m_timeSumMs = 0.0;       ///< 累计耗时(ms)
};

#endif // ZCAWHITENING_H
