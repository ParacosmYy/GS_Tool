/**
 * @file EnvelopeFollower.h
 * @brief 信号包络跟踪器 — 攻击/释放参数控制
 *
 * 功能: 实时跟踪信号包络, 支持峰值检测和RMS两种模式,
 *       可调攻击(attack)/释放(release)时间常数, 用于
 *       音频处理和信号分析中的幅度跟踪。
 *
 * 协作: PeakDetector(峰值检测) / WaveformGenerator(波形生成)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 信号包络跟踪器
 *
 * 包络跟踪器使用一阶IIR低通滤波器跟踪信号幅度:
 * - 攻击系数: 控制包络上升速度
 * - 释放系数: 控制包络下降速度
 *
 * 支持两种模式:
 * - 峰值模式: 跟踪|signal|的包络
 * - RMS模式: 跟踪sqrt(signal^2)的包络(平方/开方)
 */
class EnvelopeFollower : public QObject
{
    Q_OBJECT

public:
    /** @brief 跟踪模式 */
    enum class Mode {
        Peak,       ///< 峰值包络跟踪
        RMS,        ///< RMS包络跟踪
        PeakHold,   ///< 峰值保持(最大值保持+衰减)
        LogPeak     ///< 对数(dB)峰值跟踪
    };
    Q_ENUM(Mode)

    /** @brief 配置参数 */
    struct Config {
        Mode mode = Mode::Peak;             ///< 跟踪模式
        double attackTimeMs = 1.0;          ///< 攻击时间(ms)
        double releaseTimeMs = 50.0;        ///< 释放时间(ms)
        double sampleRate = 44100.0;        ///< 采样率(Hz)
        double holdTimeMs = 0.0;            ///< 保持时间(ms, PeakHold模式)
    };

    /** @brief 包络分析结果 */
    struct EnvelopeResult {
        QVector<double> envelope;           ///< 包络曲线
        double peakValue = 0.0;             ///< 峰值
        double rmsValue = 0.0;              ///< RMS值
        double crestFactor = 0.0;           ///< 波峰因子(峰值/RMS)
        double dynamicRange = 0.0;          ///< 动态范围(dB)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesProcessed = 0;      ///< 累计处理样本数
        int totalBuffersProcessed = 0;      ///< 累计处理缓冲区数
        double peakEnvelope = 0.0;          ///< 峰值包络
        double minEnvelope = 1e18;          ///< 最小包络
        int totalAttacks = 0;               ///< 攻击事件数
        int totalReleases = 0;              ///< 释放事件数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit EnvelopeFollower(QObject* parent = nullptr);

    /**
     * @brief 配置包络跟踪器
     * @param config 配置参数
     */
    void configure(const Config& config);

    /**
     * @brief 处理单个样本
     * @param sample 输入样本值
     * @return 当前包络值
     */
    double processSample(double sample);

    /**
     * @brief 处理样本缓冲区
     * @param samples 输入样本数组
     * @return 包络分析结果
     */
    EnvelopeResult processBuffer(const QVector<double>& samples);

    /**
     * @brief 实时检测信号超过阈值
     * @param threshold 阈值
     * @param samples 输入样本
     * @return 超过阈值的位置索引列表
     */
    QVector<int> detectAboveThreshold(double threshold,
                                      const QVector<double>& samples);

    /**
     * @brief 获取当前包络值
     * @return 当前包络幅度
     */
    double currentEnvelope() const;

    /**
     * @brief 重置跟踪器状态
     */
    void reset();

    /**
     * @brief 计算攻击系数
     * @param attackTimeMs 攻击时间(ms)
     * @param sampleRate 采样率
     * @return IIR系数
     */
    static double calcAttackCoeff(double attackTimeMs, double sampleRate);

    /**
     * @brief 计算释放系数
     * @param releaseTimeMs 释放时间(ms)
     * @param sampleRate 采样率
     * @return IIR系数
     */
    static double calcReleaseCoeff(double releaseTimeMs, double sampleRate);

    Config config() const;
    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 包络超过阈值 @param value 包络值 @param threshold 阈值 */
    void thresholdExceeded(double value, double threshold);

    /** @brief 缓冲区处理完成 @param peakValue 峰值 @param rmsValue RMS值 */
    void bufferProcessed(double peakValue, double rmsValue);

private:
    /**
     * @brief 更新峰值模式包络
     * @param input 输入绝对值
     * @return 新的包络值
     */
    double updatePeak(double input);

    /**
     * @brief 更新RMS模式包络
     * @param input 输入样本
     * @return 新的RMS包络值
     */
    double updateRMS(double input);

    /**
     * @brief 更新峰值保持模式
     * @param input 输入绝对值
     * @return 新的保持值
     */
    double updatePeakHold(double input);

    Config m_config;
    double m_envelope = 0.0;        ///< 当前包络值
    double m_rmsAccum = 0.0;        ///< RMS累加器
    double m_holdValue = 0.0;       ///< 峰值保持值
    int m_holdCounter = 0;          ///< 保持计数器
    double m_attackCoeff = 0.0;     ///< 攻击系数
    double m_releaseCoeff = 0.0;    ///< 释放系数

    Stats m_stats;
    double m_timeSum = 0.0;
};
