/**
 * @file DynamicCompressor.h
 * @brief 动态压缩器 — 音频动态范围处理
 *
 * 功能: 支持压缩/扩展/噪声门/限幅/多频段处理，
 *       提供attack/release时间常数、 Knee软拐点、
 *       增益自动补偿，统计处理帧数/平均增益/平均处理耗时。
 */
#ifndef DYNAMICCOMPRESSOR_H
#define DYNAMICCOMPRESSOR_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class DynamicCompressor
 * @brief 动态范围压缩/扩展器，支持多频段处理
 */
class DynamicCompressor : public QObject {
    Q_OBJECT
public:
    /** 处理模式 */
    enum class Mode {
        Compressor,     ///< 压缩器
        Expander,       ///< 扩展器
        NoiseGate,      ///< 噪声门
        Limiter         ///< 限幅器
    };

    /** 频段参数 */
    struct BandParam {
        double lowFreq;             ///< 频段下边界(Hz)
        double highFreq;            ///< 频段上边界(Hz)
        double threshold;           ///< 阈值(dB)
        double ratio;               ///< 压缩比
        double attackMs;            ///< 启动时间(ms)
        double releaseMs;           ///< 释放时间(ms)
        double kneeWidth;           ///< 软拐点宽度(dB)
        double makeupGain;          ///< 补偿增益(dB)
    };

    /** 处理结果 */
    struct ProcessResult {
        QVector<double> output;         ///< 输出采样
        double peakInputDb;             ///< 峰值输入(dB)
        double peakOutputDb;            ///< 峰值输出(dB)
        double gainReductionDb;         ///< 最大增益削减(dB)
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalFrames = 0;             ///< 总处理帧数
        quint64 totalSamples = 0;            ///< 累计采样点数
        double  avgGainReductionDb = 0.0;    ///< 平均增益削减(dB)
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit DynamicCompressor(QObject* parent = nullptr);

    /** 设置全局参数 */
    void setSampleRate(double rate);
    void setMode(Mode mode);
    void setThreshold(double thresholdDb);
    void setRatio(double ratio);
    void setAttack(double attackMs);
    void setRelease(double releaseMs);
    void setKneeWidth(double kneeDb);
    void setMakeupGain(double gainDb);

    /** 多频段: 设置频段参数 */
    void setBands(const QList<BandParam>& bands);

    /** 处理单帧数据 */
    ProcessResult process(const QVector<double>& input);

    /** 多频段独立处理 */
    ProcessResult processMultiband(const QVector<double>& input);

    /** 处理并获取增益曲线 */
    QVector<double> gainCurve(const QVector<double>& inputDb) const;

    /** 自动计算补偿增益 */
    double computeAutoMakeupGain(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 处理完成信号 */
    void frameProcessed(double gainReductionDb, double peakDb);
    /** 削减警告信号 */
    void clippingDetected(int sampleIndex, double value);

private:
    /** 计算静态增益特征 */
    double staticCharacteristic(double inputDb) const;
    /** 包络跟随器(平滑增益变化) */
    double envelopeFollow(double inputDb, double currentGainDb);
    /** 双二阶滤波器(频段分离) */
    void applyBandFilter(const QVector<double>& input, int bandIndex,
                         QVector<double>& lowOut, QVector<double>& highOut);

    double m_sampleRate;
    Mode m_mode;
    double m_threshold;
    double m_ratio;
    double m_attackMs;
    double m_releaseMs;
    double m_kneeWidth;
    double m_makeupGain;
    QList<BandParam> m_bands;
    double m_envelopeState;
    Stats m_stats;
    double m_timeSum;
};

#endif // DYNAMICCOMPRESSOR_H
