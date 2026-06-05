/**
 * @file PitchShift.h
 * @brief 变调(Pitch Shift)处理器
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class PitchShift
 * @brief 基于相位声码器的变调处理器
 *
 * 在不改变时长的前提下改变音频的基频。
 * 使用STFT+相位连续化实现高品质变调。
 */
class PitchShift : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalProcessed = 0;    /**< 总处理次数 */
        int totalFrames = 0;       /**< 总帧数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param fftSize FFT大小(默认2048)
     * @param hopSize 跳步大小(默认512)
     * @param parent 父对象
     */
    explicit PitchShift(int fftSize = 2048, int hopSize = 512,
                         QObject* parent = nullptr);

    /**
     * @brief 变调处理
     * @param input 输入信号
     * @param semitones 半音偏移(正=升调，负=降调)
     * @return 变调后的信号
     */
    QVector<double> process(const QVector<double>& input, double semitones);

    /**
     * @brief 按比例变调
     * @param input 输入信号
     * @param ratio 频率比率(1.0=不变, 2.0=高八度)
     * @return 变调后的信号
     */
    QVector<double> processByRatio(const QVector<double>& input, double ratio);

    /** @brief 设置FFT/跳步参数 */
    void setParameters(int fftSize, int hopSize);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int inputSize, int outputSize, double semitones);

private:
    int m_fftSize;
    int m_hopSize;
    Stats m_stats;
    double m_timeSum;
};
