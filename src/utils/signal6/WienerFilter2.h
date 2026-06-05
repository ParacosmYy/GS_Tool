/**
 * @file WienerFilter2.h
 * @brief 频域维纳滤波器 — 最优降噪
 *
 * 功能: 在频域实现维纳滤波器，通过估计噪声功率谱来
 *       最优地抑制加性噪声。支持从纯噪声样本自动估计
 *       噪声谱或手动设置噪声功率。
 *
 * 协作: DigitalFilter(时域滤波) / SpectrumAnalyzer(频谱分析)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QtGlobal>

/**
 * @class WienerFilter2
 * @brief 频域维纳滤波器
 *
 * 从含噪信号中估计干净信号，基于最小均方误差准则。
 * 需要先通过estimateNoise()或setNoisePower()设置噪声
 * 功率谱，再调用process()进行滤波。
 */
class WienerFilter2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalProcessed = 0;        ///< 累计处理帧数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param fftSize FFT大小(默认512，必须是2的幂)
     * @param parent 父对象
     */
    explicit WienerFilter2(int fftSize = 512, QObject *parent = nullptr);

    /**
     * @brief 从纯噪声样本估计噪声功率谱
     * @param noise 纯噪声数据(长度应>=fftSize)
     */
    void estimateNoise(const QVector<double> &noise);

    /**
     * @brief 对含噪信号进行维纳滤波
     * @param signal 含噪信号(长度任意，内部按fftSize分帧处理)
     * @return 滤波后的信号
     */
    QVector<double> process(const QVector<double> &signal);

    /**
     * @brief 手动设置噪声功率谱
     * @param power 噪声功率谱密度(长度=fftSize/2+1)
     */
    void setNoisePower(const QVector<double> &power);

    /** @brief 设置FFT大小 @param size 必须是2的幂 */
    void setFftSize(int size);

    /** @brief 获取FFT大小 @return 当前FFT大小 */
    int fftSize() const { return m_fftSize; }

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 @param frameCount 处理帧数 @param timeMs 耗时 */
    void processCompleted(int frameCount, double timeMs);

private:
    /**
     * @brief 基2 FFT(就地)
     * @param real 实部
     * @param imag 虚部
     * @param inverse 是否逆变换
     */
    void fftImpl(QVector<double> &real, QVector<double> &imag,
                 bool inverse) const;

    /**
     * @brief 求下一个2的幂
     * @param n 输入值
     * @return >= n 的最小2的幂
     */
    static int nextPowerOf2(int n);

    int m_fftSize;                          ///< FFT大小
    QVector<double> m_noisePower;           ///< 估计的噪声功率谱
    bool m_noiseEstimated;                  ///< 噪声是否已估计
    Stats m_stats;                          ///< 统计信息
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};
