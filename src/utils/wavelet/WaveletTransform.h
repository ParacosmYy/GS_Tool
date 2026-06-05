/**
 * @file WaveletTransform.h
 * @brief 小波变换引擎 — Haar/db2/db4/CDF97离散小波变换
 *
 * 功能: 4种小波基的DWT/IDWT，支持多级分解和重构，
 *       用于信号去噪和特征提取。
 *
 * 协作: FftPipeline(频域分析) / DigitalFilter(滤波)
 */
#ifndef WAVELETTRANSFORM_H
#define WAVELETTRANSFORM_H

#include <QObject>
#include <QVector>
#include <QList>

class WaveletTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 小波类型 */
    enum class WaveletType {
        Haar,       ///< Haar小波
        DB2,        ///< Daubechies 2
        DB4,        ///< Daubechies 4
        CDF97       ///< Cohen-Daubechies-Feauveau 9/7
    };
    Q_ENUM(WaveletType)

    /** @brief 分解结果 */
    struct Decomposition {
        QVector<double> approximation;  ///< 近似系数
        QVector<double> detail;          ///< 细节系数
        int level = 0;                   ///< 分解级数
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        quint64 totalReconstructions = 0;
        quint64 totalPointsProcessed = 0;
        double  averageEnergyRatio = 0.0;
    };

    explicit WaveletTransform(QObject* parent = nullptr);

    void setWaveletType(WaveletType type);
    void setLevels(int levels);

    /** @brief 多级DWT分解 @param data 输入信号 @return 各级分解 */
    QList<Decomposition> decompose(const QVector<double>& data);

    /** @brief 单级DWT @param data 输入 @return 分解结果 */
    Decomposition decomposeOneLevel(const QVector<double>& data);

    /** @brief IDWT重构 @param decomp 分解结果 @return 重构信号 */
    QVector<double> reconstruct(const QList<Decomposition>& decomp);

    /** @brief 去噪(阈值法) @param data 信号 @param threshold 阈值 @return 去噪信号 */
    QVector<double> denoise(const QVector<double>& data, double threshold);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decomposed(int levels);
    void reconstructed(int sampleCount);

private:
    void getWaveletCoeffs(QVector<double>& lowDecomp, QVector<double>& highDecomp,
                          QVector<double>& lowRecon, QVector<double>& highRecon) const;

    WaveletType m_type;
    int m_levels;
    double m_energySum;
    Stats m_stats;
};

#endif // WAVELETTRANSFORM_H
