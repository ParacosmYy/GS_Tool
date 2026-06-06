/**
 * @file Periodogram.h
 * @brief 周期图谱估计(Bartlett/Welch方法+置信区间) — Periodogram Spectral Estimation with Bartlett/Welch Methods and Confidence Intervals
 *
 * 功能: 实现周期图谱估计，支持经典周期图、Bartlett平均周期图、
 *       Welch加窗平均周期图和统计置信区间计算。
 *
 * 协作: FftEngine(FFT引擎) / Goertzel4(Goertzel算法) / WindowFunction(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图谱估计器
 */
class Periodogram : public QObject {
    Q_OBJECT

public:
    /** @brief 估计方法 */
    enum Method { Classical = 0, Bartlett = 1, Welch = 2 };

    /** @brief 窗函数类型 */
    enum Window { Rectangular = 0, Hann = 1, Hamming = 2, Blackman = 3 };

    /** @brief 谱估计结果 */
    struct SpectrumResult {
        QVector<double> frequencies;    ///< 频率轴(Hz)
        QVector<double> psd;            ///< 功率谱密度
        QVector<double> psdDb;          ///< PSD(dB)
        double totalPower = 0.0;        ///< 总功率
        double peakFrequency = 0.0;     ///< 峰值频率
        double peakPower = 0.0;         ///< 峰值功率
        int fftSize = 0;               ///< FFT大小
    };

    /** @brief 置信区间 */
    struct ConfidenceInterval {
        double lower = 0.0;  ///< 下界(dB)
        double upper = 0.0;  ///< 上界(dB)
        double confidence = 0.0; ///< 置信水平
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEstimates = 0;        ///< 累计估计次数
        int lastFftSize = 0;               ///< 最近FFT大小
        Method lastMethod = Classical;     ///< 最近方法
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit Periodogram(QObject *parent = nullptr);
    ~Periodogram() override;

    void setMethod(Method method);
    void setWindow(Window window);
    void setSampleRate(double rate);
    void setFftSize(int size);
    void setOverlap(double overlap);

    /**
     * @brief 计算功率谱密度
     * @param signal 输入信号
     * @return 谱估计结果
     */
    SpectrumResult estimate(const QVector<double>& signal);

    /** @brief 计算置信区间 */
    ConfidenceInterval confidenceInterval(double confidenceLevel = 0.95) const;

    /** @brief 获取最近结果 */
    SpectrumResult lastResult() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(double peakFreq, double peakPower);
    void methodChanged(Method method);

private:
    /** @brief 应用窗函数 */
    QVector<double> applyWindow(const QVector<double>& data) const;

    /** @brief 计算FFT(简化DFT) */
    void computeDFT(const QVector<double>& input,
                    QVector<double>& realOut,
                    QVector<double>& imagOut) const;

    /** @brief 计算单段周期图 */
    QVector<double> singlePeriodogram(const QVector<double>& segment) const;

    /** @brief 查找峰值 */
    static QPair<double, double> findPeak(const QVector<double>& psd,
                                          const QVector<double>& freqs);

    Method m_method = Welch;
    Window m_window = Hann;
    double m_sampleRate = 44100.0;
    int m_fftSize = 1024;
    double m_overlap = 0.5;

    SpectrumResult m_lastResult;
    int m_numAverages = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
