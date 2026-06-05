/**
 * @file SpectralSubtraction.h
 * @brief 谱减法降噪(Spectral Subtraction)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class SpectralSubtraction
 * @brief 谱减法降噪 — 从含噪信号中估计并减去噪声频谱
 *
 * 先估计噪声功率谱(假设前几帧为纯噪声)，然后在频域减去噪声。
 * 支持过减参数和谱下限控制。
 */
class SpectralSubtraction : public QObject
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
     * @param fftSize FFT大小(默认512)
     * @param noiseFrames 噪声估计帧数(默认10)
     * @param parent 父对象
     */
    explicit SpectralSubtraction(int fftSize = 512, int noiseFrames = 10,
                                   QObject* parent = nullptr);

    /**
     * @brief 估计噪声谱(使用信号的前noiseFrames帧)
     * @param noiseSamples 噪声样本
     */
    void estimateNoise(const QVector<double>& noiseSamples);

    /**
     * @brief 处理信号
     * @param signal 含噪信号
     * @param overSubtraction 过减因子(默认1.5)
     * @param spectralFloor 谱下限(默认0.02)
     * @return 降噪后的信号
     */
    QVector<double> process(const QVector<double>& signal,
                             double overSubtraction = 1.5,
                             double spectralFloor = 0.02);

    /**
     * @brief 处理单帧
     * @param frame 帧数据(fftSize长度)
     * @param overSubtraction 过减因子
     * @param spectralFloor 谱下限
     * @return 降噪后的帧
     */
    QVector<double> processFrame(const QVector<double>& frame,
                                   double overSubtraction = 1.5,
                                   double spectralFloor = 0.02);

    /** @brief 设置FFT大小 */
    void setFftSize(int size);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int inputSize, int outputSize);

private:
    int m_fftSize;
    int m_noiseFrames;
    QVector<double> m_noisePower;    /**< 噪声功率谱估计 */

    Stats m_stats;
    double m_timeSum;
};
