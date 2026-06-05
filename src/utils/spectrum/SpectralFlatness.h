/**
 * @file SpectralFlatness.h
 * @brief 频谱平坦度计算器 — Wiener熵/音调噪声分类
 *
 * 功能: 计算Wiener熵(频谱平坦度)，音调与噪声分类，
 *       频谱峰值因子，频谱质心与带宽，
 *       统计计算次数/平坦度/耗时。
 */
#ifndef SPECTRALFLATNESS_H
#define SPECTRALFLATNESS_H

#include <QObject>
#include <QVector>

/**
 * @brief 频谱平坦度计算器
 */
class SpectralFlatness : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;  ///< 累计计算次数
        double  lastFlatness = 0.0;     ///< 最近平坦度
        double  lastCrestFactor = 0.0;  ///< 最近峰值因子
        double  avgFlatness = 0.0;      ///< 平均平坦度
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /** @brief 分析结果 */
    struct Result {
        double flatness = 0.0;          ///< 频谱平坦度(Wiener熵)[0,1]
        double crestFactor = 0.0;       ///< 频谱峰值因子
        double centroid = 0.0;          ///< 频谱质心(Hz)
        double bandwidth = 0.0;         ///< 频谱带宽(Hz)
        double rolloff = 0.0;           ///< 频谱滚降点(Hz)
        bool   isTonal = false;         ///< 是否为音调信号
    };

    explicit SpectralFlatness(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 从幅度谱计算平坦度 @param magnitude 幅度谱(线性) @return 分析结果 */
    Result compute(const QVector<double>& magnitude);

    /** @brief 从功率谱计算平坦度 @param powerSpectrum 功率谱 @return 平坦度 */
    double wienerEntropy(const QVector<double>& powerSpectrum);

    /** @brief 频谱峰值因子 @param magnitude 幅度谱 @return 峰值因子 */
    double spectralCrestFactor(const QVector<double>& magnitude);

    /** @brief 频谱质心 @param magnitude 幅度谱 @return 质心频率(Hz) */
    double spectralCentroid(const QVector<double>& magnitude);

    /** @brief 频谱带宽 @param magnitude 幅度谱 @return 带宽(Hz) */
    double spectralBandwidth(const QVector<double>& magnitude);

    /** @brief 频谱滚降 @param magnitude 幅度谱 @param threshold 滚降阈值(0.85) @return 滚降频率 */
    double spectralRolloff(const QVector<double>& magnitude, double threshold = 0.85);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param flatness 频谱平坦度 @param isTonal 是否为音调 */
    void computed(double flatness, bool isTonal);

private:
    double m_sampleRate;              ///< 采样率
    Stats m_stats;
    double m_timeSum;
};

#endif // SPECTRALFLATNESS_H
