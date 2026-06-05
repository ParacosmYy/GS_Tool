/**
 * @file WaveletTransform2.h
 * @brief 连续小波变换(CWT) — Morlet小波实现时频分析
 *
 * 功能: 使用Morlet小波执行连续小波变换，生成时频尺度图(scaleogram)。
 *       支持可配置的频率范围、尺度数量和采样率，输出复数系数。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / PeakDetector(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 连续小波变换引擎 — Morlet小波时频分析
 */
class WaveletTransform2 : public QObject {
    Q_OBJECT

public:
    /** @brief 小波类型 */
    enum class WaveletType {
        Morlet,             ///< Morlet小波(默认)
        Paul,               ///< Paul小波(阶数4)
        DOG                 ///< 高斯函数导数(DOG, m=2即Mexican Hat)
    };
    Q_ENUM(WaveletType)

    /** @brief CWT结果 */
    struct CwtResult {
        QVector<double> scales;                     ///< 尺度数组
        QVector<double> frequencies;                ///< 对应频率数组(Hz)
        QVector<QVector<double>> magnitude;         ///< 幅度矩阵[scale][time]
        QVector<QVector<double>> phase;             ///< 相位矩阵[scale][time]
        int timeSteps = 0;                          ///< 时间步数
        int scaleCount = 0;                         ///< 尺度数
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalTransforms = 0;            ///< 累计变换次数
        int totalSamplesProcessed = 0;      ///< 累计处理采样数
        int totalScalesComputed = 0;        ///< 累计计算尺度数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit WaveletTransform2(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置小波类型 @param type 小波类型 */
    void setWaveletType(WaveletType type);

    /** @brief 设置分析频率范围 @param minFreq 最低频率(Hz) @param maxFreq 最高频率(Hz) */
    void setFrequencyRange(double minFreq, double maxFreq);

    /** @brief 设置尺度数量 @param count 尺度数(影响频率分辨率) */
    void setScaleCount(int count);

    /** @brief 执行连续小波变换 @param signal 输入信号 @return CWT结果 */
    CwtResult transform(const QVector<double>& signal);

    /** @brief 快速CWT仅返回幅度 @param signal 输入信号 @return 幅度矩阵 */
    QVector<QVector<double>> transformMagnitude(const QVector<double>& signal);

    /** @brief 尺度转频率 @param scale 尺度 @return 频率(Hz) */
    double scaleToFrequency(double scale) const;

    /** @brief 频率转尺度 @param freq 频率(Hz) @return 尺度 */
    double frequencyToScale(double freq) const;

    /** @brief 计算Morlet小波在指定尺度的锥形影响域(COI) @param scale 尺度 @return COI时间点数 */
    int coneOfInfluence(double scale) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换进度 @param scaleIndex 当前尺度索引 @param totalScales 总尺度数 */
    void progress(int scaleIndex, int totalScales);

    /** @brief 变换完成 @param timeSteps 时间步数 @param scaleCount 尺度数 */
    void transformComplete(int timeSteps, int scaleCount);

private:
    /** @brief 生成指定尺度的Morlet小波 @param scale 尺度 @param signalLength 信号长度 @return (实部, 虚部) */
    QPair<QVector<double>, QVector<double>> morletWavelet(double scale,
                                                          int signalLength) const;

    /** @brief 生成Paul小波 @param scale 尺度 @param signalLength 信号长度 @return (实部, 虚部) */
    QPair<QVector<double>, QVector<double>> paulWavelet(double scale,
                                                        int signalLength) const;

    /** @brief 生成DOG(高斯导数)小波 @param scale 尺度 @param signalLength 信号长度 @return (实部, 虚部) */
    QPair<QVector<double>, QVector<double>> dogWavelet(double scale,
                                                       int signalLength) const;

    /** @brief 计算两个信号的点积(卷积核心) @param signal 信号 @param waveRe 小波实部 @param waveIm 小波虚部 @return (实部结果, 虚部结果) */
    QPair<double, double> convolveAt(const QVector<double>& signal,
                                     const QVector<double>& waveRe,
                                     const QVector<double>& waveIm) const;

    double m_sampleRate = 1000.0;           ///< 采样率(Hz)
    double m_minFreq = 1.0;                 ///< 最低分析频率(Hz)
    double m_maxFreq = 400.0;               ///< 最高分析频率(Hz)
    int m_scaleCount = 64;                  ///< 尺度数
    WaveletType m_waveletType = WaveletType::Morlet; ///< 小波类型

    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
