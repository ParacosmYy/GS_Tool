/**
 * @file FftPipeline.h
 * @brief FFT管道 — 多级频谱分析管线
 *
 * 功能: 支持FFT/IFFT/功率谱/相位谱/频谱图(STFT)，
 *       统计处理帧数/平均帧率/峰值频率检测次数。
 */
#ifndef FFTPIPELINE_H
#define FFTPIPELINE_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @class FftPipeline
 * @brief 多级FFT处理管道，用于串口数据的频域分析
 */
class FftPipeline : public QObject {
    Q_OBJECT
public:
    /** 窗函数类型 */
    enum class WindowFunction {
        None,       ///< 无窗
        Hamming,    ///< 汉明窗
        Hanning,    ///< 汉宁窗
        Blackman,   ///< 布莱克曼窗
        FlatTop     ///< 平顶窗
    };

    /** 频谱结果 */
    struct SpectrumResult {
        QVector<double> frequencies;    ///< 频率轴
        QVector<double> magnitude;      ///< 幅度谱
        QVector<double> phase;          ///< 相位谱
        QVector<double> power;          ///< 功率谱
        double peakFrequency = 0.0;     ///< 峰值频率
        double peakMagnitude = 0.0;     ///< 峰值幅度
        double totalPower = 0.0;        ///< 总功率
    };

    /** 管道统计 */
    struct Stats {
        quint64 totalFrames = 0;
        quint64 peakDetections = 0;
        double  averageFrameTimeMs = 0.0;
        double  totalProcessingTimeMs = 0.0;
    };

    explicit FftPipeline(QObject* parent = nullptr);

    void setSampleRate(double rate);
    void setWindowFunction(WindowFunction wf);
    void setFftSize(int size);

    /** 执行FFT */
    SpectrumResult forwardFft(const QVector<double>& data);
    /** 执行IFFT */
    QVector<double> inverseFft(const QVector<double>& real, const QVector<double>& imag);
    /** 功率谱密度 */
    QVector<double> powerSpectrum(const QVector<double>& data);
    /** 短时傅里叶变换(STFT) */
    QList<SpectrumResult> stft(const QVector<double>& data, int hopSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumReady(const SpectrumResult& result);
    void peakDetected(double frequency, double magnitude);

private:
    void applyWindow(QVector<double>& data);
    int nextPow2(int n) const;

    double m_sampleRate;
    WindowFunction m_windowFunc;
    int m_fftSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // FFTPIPELINE_H
