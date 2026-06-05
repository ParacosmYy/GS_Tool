/**
 * @file SpectralFeatures.h
 * @brief 频谱特征提取 — 质心/带宽/滚降/平坦度
 *
 * 功能: 从频谱数据中提取常用音频/信号特征，包括频谱质心(Spectral
 *       Centroid)、频谱带宽(Spectral Bandwidth)、频谱滚降(Spectral
 *       Rolloff)和频谱平坦度(Spectral Flatness)。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / MfccExtractor(MFCC特征)
 */
#ifndef SPECTRALFEATURES_H
#define SPECTRALFEATURES_H

#include <QObject>
#include <QVector>

/**
 * @brief 频谱特征提取 — 质心/带宽/滚降/平坦度
 */
class SpectralFeatures : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit SpectralFeatures(QObject* parent = nullptr);

    /** @brief 频谱质心(加权平均频率)
     *  @param spectrum 幅度谱
     *  @param freqs 对应频率数组
     *  @return 质心频率(Hz) */
    double centroid(const QVector<double>& spectrum,
                    const QVector<double>& freqs);

    /** @brief 频谱带宽(质心附近二阶矩)
     *  @param spectrum 幅度谱
     *  @param freqs 对应频率数组
     *  @return 带宽(Hz) */
    double bandwidth(const QVector<double>& spectrum,
                     const QVector<double>& freqs);

    /** @brief 频谱滚降(能量累积百分比对应频率)
     *  @param spectrum 幅度谱
     *  @param freqs 对应频率数组
     *  @param pct 能量百分比(默认85%)
     *  @return 滚降频率(Hz) */
    double rolloff(const QVector<double>& spectrum,
                   const QVector<double>& freqs,
                   double pct = 0.85);

    /** @brief 频谱平坦度(几何均值/算术均值)
     *  @param spectrum 幅度谱
     *  @return 平坦度[0,1]，越接近1越平坦(噪声) */
    double flatness(const QVector<double>& spectrum);

    /** @brief 一次性提取所有特征
     *  @param spectrum 幅度谱
     *  @param freqs 对应频率数组
     *  @return 特征向量(质心,带宽,滚降,平坦度) */
    QVector<double> extractAll(const QVector<double>& spectrum,
                               const QVector<double>& freqs);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 特征计算完成 @param numFeatures 特征数量 */
    void computationCompleted(int numFeatures);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // SPECTRALFEATURES_H
