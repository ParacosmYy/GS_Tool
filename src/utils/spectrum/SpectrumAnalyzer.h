/**
 * @file SpectrumAnalyzer.h
 * @brief 频谱分析引擎 — DFT/FFT计算频域特征
 *
 * 功能: 支持DFT和基2 FFT，计算幅度谱/功率谱/相位谱，
 *       提取峰值频率/频谱质心/带宽等特征。
 *
 * 协作: FftEngine(FFT引擎) / ChartWidget(频谱显示)
 */
#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 频谱分析引擎 — 频域特征提取
 */
class SpectrumAnalyzer : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数 */
    enum class WindowFunction {
        None,       ///< 无窗(矩形窗)
        Hanning,    ///< 汉宁窗
        Hamming,    ///< 海明窗
        Blackman    ///< 布莱克曼窗
    };
    Q_ENUM(WindowFunction)

    /** @brief 频谱峰值 */
    struct Peak {
        double frequency = 0.0;     ///< 频率(Hz)
        double magnitude = 0.0;     ///< 幅度
        double phase = 0.0;         ///< 相位(rad)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalFFTsComputed = 0;      ///< 累计FFT计算次数
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样数
        double  averageProcessingTimeMs = 0.0;///< 平均处理时间(ms)
        double  peakFrequency = 0.0;        ///< 历史峰值频率
    };

    explicit SpectrumAnalyzer(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置窗函数 @param window 窗函数 */
    void setWindowFunction(WindowFunction window);

    /** @brief 计算频谱 @param data 时域数据 @return (频率数组, 幅度数组) */
    QPair<QVector<double>, QVector<double>> computeSpectrum(
        const QVector<double>& data);

    /** @brief 查找频谱峰值 @param magnitude 幅度谱 @param threshold 峰值阈值 @return 峰值列表 */
    QList<Peak> findPeaks(const QVector<double>& magnitude,
                          double threshold = 0.1) const;

    /** @brief 计算频谱质心 @param frequencies 频率 @param magnitude 幅度 @return 质心频率 */
    double spectralCentroid(const QVector<double>& frequencies,
                            const QVector<double>& magnitude) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 频谱计算完成 @param sampleCount 采样数 @param binCount 频率bin数 */
    void spectrumReady(int sampleCount, int binCount);

private:
    void applyWindow(QVector<double>& data);
    void fft(QVector<double>& real, QVector<double>& imag);
    QVector<double> generateWindow(int size) const;

    double m_sampleRate;            ///< 采样率
    WindowFunction m_windowFunc;    ///< 窗函数

    Stats m_stats;
    double m_timeSum;               ///< 处理时间累加器
};

#endif // SPECTRUMANALYZER_H
