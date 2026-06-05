/**
 * @file MultiTaper3.h
 * @brief 多锥谱3 — 自适应DPSS+多窗加权
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class MultiTaper3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEstimates = 0;
        int totalSamplesProcessed = 0;
        int numTapers = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultiTaper3(QObject* parent = nullptr);

    void setParameters(int fftSize, int numTapers, double nw = 4.0);
    void setSampleRate(double sampleRate);
    QVector<double> estimate(const QVector<double>& signal);
    QVector<double> powerSpectrum() const { return m_psd; }
    QVector<double> frequencies() const;

    int fftSize() const { return m_fftSize; }
    int numTapers() const { return m_numTapers; }
    double bandwidth() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(int numTapers, double bandwidth);

private:
    int m_fftSize = 1024;
    int m_numTapers = 5;
    double m_nw = 4.0;
    double m_sampleRate = 44100.0;
    QVector<double> m_psd;
    QVector<QVector<double>> m_tapers;
    QVector<double> m_eigenvalues;
    bool m_initialized = false;

    void computeDPSS();
    QVector<double> tridiagEigen(int n, double a, double b) const;
    void fft(QVector<double>& real, QVector<double>& imag, int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
