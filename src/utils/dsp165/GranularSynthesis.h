/**
 * @file GranularSynthesis.h
 * @brief 颗粒合成器(粒子云+位置散射+包络) — Granular Synthesizer with Grain Cloud, Position Scatter and Envelope Shaping
 *
 * 功能: 实现颗粒合成引擎，支持多粒子并行播放、位置随机散射、
 *       多种包络形状(Hanning/Gaussian/Trapezoid)、粒子密度和时移控制。
 *
 * 协作: PitchDetector(基频检测) / GranularSynthesis(时域合成) / FftEngine(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 颗粒合成器
 */
class GranularSynthesis : public QObject {
    Q_OBJECT

public:
    /** @brief 包络类型 */
    enum Envelope { Hanning, Gaussian, Trapezoid };

    /** @brief 单个粒子参数 */
    struct Grain {
        double position;        ///< 源文件中的起始位置(采样点)
        double startTime;       ///< 输出时间偏移(秒)
        int duration;           ///< 粒子长度(采样点)
        double pitch;           ///< 播放速率(1.0=原始)
        double amplitude;       ///< 振幅缩放
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSyntheses = 0;         ///< 累计合成次数
        int lastGrainCount = 0;             ///< 最近粒子数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit GranularSynthesis(QObject* parent = nullptr);
    ~GranularSynthesis() override;

    /** @brief 设置采样率 */
    void setSampleRate(int rate);

    /** @brief 设置粒子持续时间(ms) */
    void setGrainDuration(double ms);

    /** @brief 设置粒子密度(粒子/秒) */
    void setGrainDensity(double density);

    /** @brief 设置位置散射范围(0~1) */
    void setPositionScatter(double scatter);

    /** @brief 设置包络类型 */
    void setEnvelope(Envelope env);

    /**
     * @brief 从源音频生成粒子云合成
     * @param source 输入音频采样
     * @param outputLength 输出长度(采样点)
     * @return 合成后的音频
     */
    QVector<double> synthesize(const QVector<double>& source, int outputLength);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 合成完成 @param grainCount 粒子数 */
    void synthesisCompleted(int grainCount);

private:
    /** @brief 生成粒子云 */
    QVector<Grain> generateGrainCloud(int sourceLen, int outputLen) const;

    /** @brief 计算包络 */
    QVector<double> computeEnvelope(int grainLen) const;

    /** @brief 提取带包络的粒子 */
    void extractGrain(const QVector<double>& source, const Grain& g,
                      QVector<double>& out) const;

    int m_sampleRate = 44100;
    double m_grainDuration = 50.0;     ///< ms
    double m_grainDensity = 20.0;      ///< grains/sec
    double m_positionScatter = 0.3;
    Envelope m_envelope = Hanning;

    Stats m_stats;
    double m_timeSum = 0.0;
};
