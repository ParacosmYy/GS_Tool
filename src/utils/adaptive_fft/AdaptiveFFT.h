/**
 * @file AdaptiveFFT.h
 * @brief 自适应FFT频谱分析器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 自适应FFT频谱分析器
 *
 * 自动选择FFT大小,支持频谱泄漏补偿、
 * 窗函数应用、峰值检测和频率估计。
 */
class AdaptiveFFT : public QObject
{
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum WindowType {
        Rectangular = 0, ///< 矩形窗
        Hanning = 1,     ///< 汉宁窗
        Hamming = 2,     ///< 汉明窗
        Blackman = 3,    ///< 布莱克曼窗
        FlatTop = 4      /// 平顶窗
    };
    Q_ENUM(WindowType)

    /** @brief 频谱峰值 */
    struct Peak {
        double frequency = 0.0;  ///< 频率(Hz)
        double magnitude = 0.0;  ///< 幅值
        double phase = 0.0;      ///< 相位(rad)
        double snr = 0.0;        ///< 信噪比(dB)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;     ///< 总变换次数
        int totalPeaksDetected = 0;  ///< 总检测峰值数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit AdaptiveFFT(QObject* parent = nullptr);

    /**
     * @brief 执行自适应FFT分析
     * @param data 输入信号
     * @param sampleRate 采样率(Hz)
     * @param window 窗函数类型
     * @return 频谱幅值数组
     */
    QVector<double> analyze(const QVector<double>& data, double sampleRate,
                            WindowType window = Hanning);

    /**
     * @brief 检测频谱峰值
     * @param magnitude 频谱幅值
     * @param sampleRate 采样率
     * @param thresholdDb 峰值阈值(dB)
     * @return 峰值列表
     */
    QVector<Peak> detectPeaks(const QVector<double>& magnitude,
                              double sampleRate, double thresholdDb = -40.0);

    /**
     * @brief 精确频率估计(抛物线插值)
     * @param magnitude 频谱幅值
     * @param peakIndex 峰值索引
     * @param sampleRate 采样率
     * @param fftSize FFT大小
     * @return 精确频率(Hz)
     */
    double estimateFrequency(const QVector<double>& magnitude,
                             int peakIndex, double sampleRate, int fftSize);

    /**
     * @brief 生成窗函数
     * @param size 窗口大小
     * @param type 窗函数类型
     * @return 窗系数
     */
    QVector<double> generateWindow(int size, WindowType type);

    /**
     * @brief 计算功率谱密度
     * @param data 输入信号
     * @param sampleRate 采样率
     * @return {频率数组, PSD值数组}
     */
    QPair<QVector<double>, QVector<double>> powerSpectralDensity(
        const QVector<double>& data, double sampleRate);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 分析完成信号 */
    void analysisCompleted(int fftSize, int peakCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    int nextPowerOf2(int n) const;
    void inPlaceFFT(QVector<double>& real, QVector<double>& imag);
};
