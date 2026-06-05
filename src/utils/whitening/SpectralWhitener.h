/**
 * @file SpectralWhitener.h
 * @brief 频谱白化 — FFT频谱平坦化处理
 *
 * 功能: 对信号进行频谱白化处理，使频谱趋于平坦，
 *       支持白化强度控制、频谱平滑，统计处理次数/耗时。
 */
#ifndef SPECTRALWHITENER_H
#define SPECTRALWHITENER_H

#include <QObject>
#include <QVector>

class SpectralWhitener : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalWhitening = 0;
        quint64 totalSamples = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SpectralWhitener(QObject* parent = nullptr);

    /** @brief 频谱白化 @param input 输入信号 @param strength 白化强度[0,1] @return 白化后信号 */
    QVector<double> whiten(const QVector<double>& input, double strength = 0.9);

    /** @brief 获取幅度谱 @param input 输入信号 @return 幅度谱 */
    QVector<double> magnitudeSpectrum(const QVector<double>& input) const;

    /** @brief 获取功率谱 @param input 输入信号 @return 功率谱 */
    QVector<double> powerSpectrum(const QVector<double>& input) const;

    void setFFTSize(int size);
    int fftSize() const { return m_fftSize; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void whiteningCompleted(int inputSize, double flatness);

private:
    /** @brief FFT实现(基2 Cooley-Tukey) */
    void fft(QVector<double>& real, QVector<double>& imag) const;
    /** @brief IFFT实现 */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    int m_fftSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // SPECTRALWHITENER_H
