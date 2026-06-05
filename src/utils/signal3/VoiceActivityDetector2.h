/**
 * @file VoiceActivityDetector2.h
 * @brief 高级语音活动检测器 — 基于噪声估计与信噪比的VAD
 *
 * 实现基于能量阈值、噪声_floor自适应估计和信噪比(SNR)的
 * 语音活动检测算法，适用于嵌入式音频调试、串口语音流分析等场景。
 * 支持hangover帧机制减少语音尾部截断。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @class VoiceActivityDetector2
 * @brief 高级语音活动检测器 — 自适应噪声估计 + SNR判定
 *
 * 典型用法:
 * @code
 *   VoiceActivityDetector2 vad(256, 16000);
 *   vad.setNoiseThreshold(1.5);
 *   vad.setHangoverFrames(10);
 *   bool isVoiced = vad.process(audioFrame);
 * @endcode
 */
class VoiceActivityDetector2 : public QObject {
    Q_OBJECT

public:
    /** @brief 检测器运行统计结构 */
    struct Stats {
        quint64 totalFrames       = 0;   ///< 处理帧总数
        quint64 voicedFrames      = 0;   ///< 检测为语音的帧数
        double  avgProcessingTimeMs = 0.0; ///< 平均每帧处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param frameSize  每帧采样点数(默认256)
     * @param sampleRate 采样率Hz(默认16000)
     * @param parent     父对象
     */
    explicit VoiceActivityDetector2(int frameSize = 256,
                                    int sampleRate = 16000,
                                    QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~VoiceActivityDetector2() override;

    // ── 核心检测 ──

    /**
     * @brief 处理一帧音频数据，返回是否为语音帧
     *
     * 计算帧能量、更新噪声估计、计算SNR，综合判定语音活动。
     * hangover帧机制确保语音尾部不会被过早截断。
     * @param frame 音频采样数据(长度应等于frameSize)
     * @return true 表示当前帧包含语音活动
     */
    bool process(const QVector<double>& frame);

    // ── 参数配置 ──

    /**
     * @brief 设置噪声门限倍数
     * @param threshold 噪声门限倍数(默认2.0, 帧能量需超过noiseLevel*threshold)
     */
    void setNoiseThreshold(double threshold);

    /**
     * @brief 设置hangover帧数(语音结束后的延迟帧数)
     * @param frames hangover帧数(默认10, 减少语音尾部截断)
     */
    void setHangoverFrames(int frames);

    // ── 状态查询 ──

    /** @brief 获取当前噪声估计水平 @return 噪声能量估计值 */
    double noiseLevel() const;

    /** @brief 获取最近一帧的信噪比 @return SNR值(dB) */
    double snr() const;

    /** @brief 获取帧大小 @return 每帧采样点数 */
    int frameSize() const;

    /** @brief 获取采样率 @return 采样率Hz */
    int sampleRate() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器和内部状态归零 */
    void resetStatistics();

signals:
    /** @brief 语音活动开始信号(从静音切换到语音) */
    void voiceStarted();
    /** @brief 语音活动结束信号(从语音切换到静音) */
    void voiceStopped();
    /** @brief 每帧处理完成信号 @param isVoiced 是否为语音帧 @param energy 帧能量 @param snrDb 信噪比dB */
    void frameProcessed(bool isVoiced, double energy, double snrDb);

private:
    /**
     * @brief 计算帧RMS能量
     * @param frame 音频帧数据
     * @return RMS能量值
     */
    double computeEnergy(const QVector<double>& frame) const;

    /**
     * @brief 更新噪声估计(指数移动平均)
     * @param energy 当前帧能量
     * @param isVoiced 当前帧是否为语音
     */
    void updateNoiseEstimate(double energy, bool isVoiced);

    int    m_frameSize;             ///< 每帧采样点数
    int    m_sampleRate;            ///< 采样率Hz
    double m_noiseThreshold;        ///< 噪声门限倍数
    int    m_hangoverFrames;        ///< hangover总帧数
    int    m_hangoverRemaining;     ///< 当前剩余hangover帧数

    double m_noiseLevel;            ///< 噪声能量估计(指数移动平均)
    double m_snr;                   ///< 最近一帧的SNR(dB)
    double m_lastEnergy;            ///< 最近一帧的能量值
    bool   m_wasVoiced;             ///< 上一帧的语音状态

    QElapsedTimer m_timer;          ///< 处理耗时计时器
    Stats  m_stats;                 ///< 统计数据
};
