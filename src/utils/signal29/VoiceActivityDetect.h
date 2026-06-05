/**
 * @file VoiceActivityDetect.h
 * @brief 语音活动检测 — 能量/ZCR/谱熵/自适应阈值
 *
 * 功能: 检测音频信号中的语音/静音段，支持短时能量、过零率、
 *       谱熵三种特征融合，自适应阈值更新，适合嵌入式调试场景。
 *
 * 协作: DigitalFilter(预处理) / SpectrumAnalyzer(频域特征)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 语音活动检测 — 多特征融合VAD
 */
class VoiceActivityDetect : public QObject {
    Q_OBJECT

public:
    /** @brief 检测特征 */
    enum class Feature {
        Energy = 0x01,     ///< 短时能量
        ZCR    = 0x02,     ///< 过零率
        SpectralEntropy = 0x04 ///< 谱熵
    };
    Q_ENUM(Feature)

    /** @brief 检测结果 */
    struct VadResult {
        bool isVoice = false;        ///< 是否为语音
        double energy = 0.0;         ///< 短时能量
        double zcr = 0.0;            ///< 过零率
        double spectralEntropy = 0.0;///< 谱熵
        double confidence = 0.0;     ///< 置信度(0~1)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalFramesProcessed = 0;  ///< 累计处理帧数
        quint64 totalVoiceFrames = 0;      ///< 累计语音帧数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        double  avgEnergy = 0.0;           ///< 平均能量
        double  avgZcr = 0.0;              ///< 平均过零率
    };

    explicit VoiceActivityDetect(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置帧长 @param samples 采样数(默认256) */
    void setFrameSize(int samples);

    /** @brief 设置使用的特征组合 @param features 特征位掩码 */
    void setFeatures(int features);

    /** @brief 设置初始能量阈值 @param threshold 阈值 */
    void setEnergyThreshold(double threshold);

    /** @brief 设置自适应因子 @param alpha 平滑因子(0~1) */
    void setAdaptiveAlpha(double alpha);

    /** @brief 检测单帧 @param frame 音频帧 @return VAD结果 */
    VadResult detect(const QVector<double>& frame);

    /** @brief 批量检测 @param audio 完整音频 @return 逐帧结果列表 */
    QVector<VadResult> detectBatch(const QVector<double>& audio);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 语音段检测到 @param frameIndex 帧索引 @param energy 能量 */
    void voiceDetected(int frameIndex, double energy);

    /** @brief 静音段检测到 @param frameIndex 帧索引 */
    void silenceDetected(int frameIndex);

private:
    double computeEnergy(const QVector<double>& frame) const;
    double computeZCR(const QVector<double>& frame) const;
    double computeSpectralEntropy(const QVector<double>& frame) const;
    void updateThresholds(const VadResult& result);

    double m_sampleRate = 16000.0;     ///< 采样率
    int m_frameSize = 256;             ///< 帧长
    int m_features = 0x07;             ///< 特征掩码(默认全开)

    double m_energyThreshold = 0.01;   ///< 能量阈值
    double m_zcrThreshold = 0.3;       ///< 过零率阈值
    double m_entropyThreshold = 0.8;   ///< 谱熵阈值
    double m_alpha = 0.95;             ///< 自适应平滑因子

    double m_bgEnergy = 0.001;         ///< 背景能量估计
    double m_bgEntropy = 0.5;          ///< 背景谱熵估计
    int m_frameIndex = 0;              ///< 当前帧索引

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_energySum = 0.0;
    double m_zcrSum = 0.0;
};
