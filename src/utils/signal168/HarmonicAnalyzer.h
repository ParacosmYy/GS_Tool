/**
 * @file HarmonicAnalyzer.h
 * @brief 谐波分析(基频检测+泛音跟踪FFT峰值提取) — Harmonic Analysis with Fundamental Detection and Partial Tracking via FFT Peak Picking
 *
 * 功能: 实现谐波分析，通过FFT频谱峰值提取检测基频，
 *       跟踪泛音序列并提取幅度/相位信息。适用于音频和振动信号分析。
 *
 * 协作: Deconvolver(反卷积) / FftEngine(FFT) / WalshHadamard(变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谐波分析器
 */
class HarmonicAnalyzer : public QObject {
    Q_OBJECT

public:
    /** @brief 谐波分量 */
    struct Harmonic {
        double frequency;          ///< 频率(Hz)
        double amplitude;          ///< 幅度
        double phase;             ///< 相位(弧度)
        int binIndex;             ///< FFT频点索引
    };

    /** @brief 分析结果 */
    struct AnalysisResult {
        double fundamentalFreq;    ///< 基频(Hz)
        double fundamentalAmp;     ///< 基频幅度
        QVector<Harmonic> harmonics; ///< 所有检测到的谐波
        double thd;               ///< 总谐波失真(THD)
        double thdPlusNoise;      ///< THD+N
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalAnalyses = 0;      ///< 累计分析次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        double lastFundamental = 0.0;    ///< 最近基频(Hz)
    };

    explicit HarmonicAnalyzer(QObject* parent = nullptr);
    ~HarmonicAnalyzer() override;

    /** @brief 设置采样率 */
    void setSampleRate(double rate);

    /** @brief 设置FFT大小 */
    void setFFTSize(int size);

    /** @brief 设置最大谐波阶数 */
    void setMaxHarmonics(int maxH);

    /**
     * @brief 分析信号谐波结构
     * @param signal 时域信号
     * @return 谐波分析结果
     */
    AnalysisResult analyze(const QVector<double>& signal);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成 @param fundamental 基频 */
    void analysisCompleted(double fundamental);

private:
    /** @brief FFT(基2) */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 汉宁窗 */
    QVector<double> hanningWindow(int n) const;

    /** @brief 峰值检测 */
    QVector<int> findPeaks(const QVector<double>& magnitude, double threshold) const;

    /** @brief 抛物线插值精确定位峰值频率 */
    double interpolatePeak(const QVector<double>& magnitude, int bin) const;

    /** @brief 补零到2的幂 */
    static int nextPowerOf2(int n);

    double m_sampleRate = 44100.0;
    int m_fftSize = 4096;
    int m_maxHarmonics = 16;

    Stats m_stats;
    double m_timeSum = 0.0;
};
