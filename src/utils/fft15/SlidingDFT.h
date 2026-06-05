/**
 * @file SlidingDFT.h
 * @brief 滑动DFT — 高效单bin逐样本更新 + 指数窗 + 幅度跟踪
 *
 * 功能: 实现滑动离散傅里叶变换，每输入一个新样本仅需O(1)更新指定频bin。
 *       支持指数窗衰减、多bin同时跟踪、幅度/相位实时输出、峰值检测。
 *       统计更新次数/峰值检测数/平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class SlidingDFT
 * @brief 逐样本更新的滑动DFT处理器
 */
class SlidingDFT : public QObject {
    Q_OBJECT
public:
    /** 单个频bin的状态 */
    struct BinState {
        double real = 0.0;      ///< 实部
        double imag = 0.0;      ///< 虚部
        double magnitude = 0.0; ///< 幅度
        double phase = 0.0;     ///< 相位(弧度)
    };

    /** 处理统计 */
    struct Stats {
        quint64 totalUpdates = 0;          ///< 总更新次数
        quint64 totalPeakDetections = 0;   ///< 累计峰值检测数
        quint64 totalSamplesProcessed = 0; ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param fftSize DFT大小N(必须是2的幂)
     * @param sampleRate 采样率(Hz)
     * @param parent 父对象
     */
    explicit SlidingDFT(int fftSize = 256, double sampleRate = 44100.0,
                        QObject* parent = nullptr);

    /** @brief 设置DFT大小 @param size 必须是2的幂 */
    void setFFTSize(int size);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置指数窗衰减因子 @param alpha 衰减系数(0~1, 越小衰减越快) */
    void setAlpha(double alpha);

    /**
     * @brief 推入一个新样本并更新所有跟踪bin
     * @param sample 输入样本
     */
    void pushSample(double sample);

    /**
     * @brief 批量推入样本
     * @param samples 输入样本序列
     */
    void pushSamples(const QVector<double>& samples);

    /**
     * @brief 获取指定bin的状态
     * @param binIndex 频率索引(0 ~ N/2)
     * @return bin状态
     */
    BinState binAt(int binIndex) const;

    /** @brief 获取所有跟踪bin的幅度谱 */
    QVector<double> magnitudeSpectrum() const;

    /** @brief 检测当前频谱峰值 @param threshold 峰值阈值(相对最大幅度) */
    QVector<QPair<int, double>> detectPeaks(double threshold = 0.1) const;

    /** @brief 获取当前所有bin状态 */
    const QVector<BinState>& allBins() const { return m_bins; }

    /** @brief 获取DFT大小 */
    int fftSize() const { return m_fftSize; }

    /** @brief 获取频率分辨率(Hz/bin) */
    double frequencyResolution() const;

    /** @brief 将bin索引转换为频率(Hz) */
    double binToFrequency(int bin) const;

    /** @brief 将频率(Hz)转换为最近的bin索引 */
    int frequencyToBin(double freq) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 峰值检测 @param bin 频率bin @param freqHz 频率(Hz) @param mag 幅度 */
    void peakDetected(int bin, double freqHz, double mag);
    /** @brief 样本处理完成 @param count 累计样本数 */
    void samplesProcessed(quint64 count);

private:
    /** 预计算旋转因子 */
    void precomputeTwiddles();
    /** 重新初始化所有bin */
    void reinitBins();

    int    m_fftSize;        ///< DFT大小N
    double m_sampleRate;     ///< 采样率
    double m_alpha;          ///< 指数窗衰减因子

    /** 旋转因子 e^{-j*2*pi*k/N} */
    QVector<double> m_twiddleReal;
    QVector<double> m_twiddleImag;

    QVector<BinState> m_bins; ///< 所有bin状态
    double m_prevSample;      ///< 上一个样本(用于差分)

    mutable Stats  m_stats;         ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
