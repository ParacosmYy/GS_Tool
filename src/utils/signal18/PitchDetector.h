/**
 * @file PitchDetector.h
 * @brief 音高检测器 — YIN+自相关混合算法
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 音高检测器
 * 结合YIN算法和自相关法，支持实时逐帧检测
 */
class PitchTracker : public QObject
{
    Q_OBJECT

public:
    /** @brief 检测算法 */
    enum Method {
        YinMethod,           ///< YIN差分函数法
        Autocorrelation,     ///< 自相关法
        Hybrid               ///< 混合(默认)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalFrames = 0;               ///< 累计处理帧数
        int totalVoiced = 0;               ///< 累计有声帧数
        int totalUnvoiced = 0;             ///< 累计无声帧数
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 帧检测结果 */
    struct FrameResult {
        double frequency = 0.0;       ///< 检测频率(Hz), 0=无声
        double confidence = 0.0;      ///< 置信度[0,1]
        bool voiced = false;          ///< 是否有声
        double rms = 0.0;             ///< 均方根能量
    };

    /**
     * @brief 构造函数
     * @param sampleRate 采样率(Hz)
     * @param minFreq 最低检测频率(Hz)
     * @param maxFreq 最高检测频率(Hz)
     * @param parent 父对象
     */
    explicit PitchTracker(double sampleRate = 44100.0,
                           double minFreq = 50.0, double maxFreq = 2000.0,
                           QObject* parent = nullptr);

    /** @brief 检测单帧音高 @param frame 音频帧 @param method 检测算法 */
    FrameResult detect(const QVector<double>& frame, Method method = Hybrid);

    /** @brief 批量检测 @param signal 完整信号 @param frameSize 帧大小 @param hopSize 步长 @return 每帧结果 */
    QVector<FrameResult> detectBatch(const QVector<double>& signal,
                                     int frameSize, int hopSize);

    /** @brief 获取频率直方图(所有有声帧) @param bins 直方图bin数 */
    QVector<QPair<double,int>> frequencyHistogram(int bins = 50) const;

    /** @brief 设置检测阈值 @param threshold 有声/无声阈值[0,1] */
    void setVoicingThreshold(double threshold);

    /** @brief 获取采样率 */
    double sampleRate() const { return m_sampleRate; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 帧检测完成 @param freq 频率 @param confidence 置信度 */
    void frameDetected(double freq, double confidence);

private:
    /** @brief YIN差分函数 */
    QVector<double> yinDifference(const QVector<double>& frame) const;

    /** @brief YIN累积均值归一化差分 */
    QVector<double> yinCMND(const QVector<double>& diff) const;

    /** @brief 自相关函数 */
    QVector<double> autocorrelation(const QVector<double>& frame) const;

    /** @brief 抛物线插值精化峰值位置 */
    double parabolicRefine(const QVector<double>& data, int idx) const;

    /** @brief 计算RMS能量 */
    double computeRMS(const QVector<double>& frame) const;

    double m_sampleRate;                ///< 采样率
    double m_minFreq;                   ///< 最低频率
    double m_maxFreq;                   ///< 最高频率
    double m_threshold = 0.2;           ///< 有声阈值
    QVector<double> m_freqHistory;      ///< 频率历史

    Stats m_stats;
    double m_timeSum = 0.0;
};
