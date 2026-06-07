/**
 * @file Periodogram2.h
 * @brief 周期图(多锥估计DPSS/Slepian锥+谐波F检验) — Periodogram with Multitaper Estimation Using DPSS/Slepian Tapers and Harmonic F-Test
 *
 * 功能: 实现周期图频谱估计，支持多锥(multitaper)估计、
 *       DPSS/Slepian锥设计、谐波F检验和自适应锥选择。
 *
 * 协作: FftEngine3(FFT引擎) / Goertzel5(Goertzel) / WelchEstimator6(Welch估计)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 周期图估计器(多锥DPSS/Slepian+谐波F检验)
 */
class Periodogram2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEstimates = 0;
        int dataLength = 0;
        int numTapers = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram2(QObject *parent = nullptr);
    ~Periodogram2() override;

    void setSampleRate(double sr);
    void setNumTapers(int nt);
    void setNw(double nw);
    void setFftSize(int N);

    /** @brief Compute standard periodogram */
    QVector<double> periodogram(const QVector<double>& data) const;

    /** @brief Compute multitaper spectrum using DPSS tapers */
    QVector<double> multitaper(const QVector<double>& data);

    /** @brief Compute harmonic F-test for line components */
    QVector<double> harmonicFTest(const QVector<double>& data);

    /** @brief Get frequency axis */
    QVector<double> frequencyAxis() const;

    /** @brief Generate DPSS/Slepian tapers */
    QVector<QVector<double>> generateDPSS(int N, int K, double NW) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(int fftSize, int tapers, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numTapers = 5;
    double m_nw = 4.0;
    int m_fftSize = 1024;

    QVector<QVector<double>> m_tapers;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Cooley-Tukey radix-2 FFT (complex: interleaved re,im) */
    QVector<double> fft(const QVector<double>& real) const;

    /** @brief Compute eigenvalues of DPSS tridiagonal system */
    QVector<double> dpssEigenvalues(int N, int K, double NW) const;

    /** @brief Apply Hanning window */
    QVector<double> hanningWindow(int N) const;
};
