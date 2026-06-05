/**
 * @file ShortTimeFourier.h
 * @brief 短时傅里叶变换(STFT) — 时频分析
 *
 * 功能: STFT短时傅里叶变换，支持窗函数选择/跳步配置，
 *       返回时间×频率的2D矩阵，统计变换次数/耗时。
 */
#ifndef SHORTTIMEFOURIER_H
#define SHORTTIMEFOURIER_H

#include <QObject>
#include <QVector>

class ShortTimeFourier : public QObject {
    Q_OBJECT
public:
    enum Window { Hanning, Hamming, Blackman, Rectangular };

    struct Stats {
        quint64 totalTransforms = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit ShortTimeFourier(QObject* parent = nullptr);

    /** @brief 执行STFT @param signal 输入信号 @param sampleRate 采样率 @return {时间帧, 频率bin, 幅度矩阵[frame][bin]} */
    QVector<QVector<double>> transform(const QVector<double>& signal,
                                        double sampleRate);

    /** @brief 配置参数 @param fftSize FFT大小 @param hopSize 跳步大小 @param window 窗函数 */
    void configure(int fftSize = 512, int hopSize = 256,
                   Window window = Hanning);

    /** @brief 获取频率轴 @param sampleRate 采样率 @return 频率数组 */
    QVector<double> frequencyAxis(double sampleRate) const;

    /** @brief 获取时间轴 @param signalLength 信号长度 @param sampleRate 采样率 @return 时间数组 */
    QVector<double> timeAxis(int signalLength, double sampleRate) const;

    int fftSize() const { return m_fftSize; }
    int hopSize() const { return m_hopSize; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frames, int freqBins);

private:
    QVector<double> createWindow(int length) const;
    void fft(QVector<double>& real, QVector<double>& imag) const;

    int m_fftSize;
    int m_hopSize;
    Window m_window;
    Stats m_stats;
    double m_timeSum;
};

#endif // SHORTTIMEFOURIER_H
