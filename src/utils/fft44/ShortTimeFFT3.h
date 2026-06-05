/**
 * @file ShortTimeFFT3.h
 * @brief 短时FFT3 — 重叠保存+相位声码器
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ShortTimeFFT3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalFrames = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ShortTimeFFT3(QObject* parent = nullptr);

    void setFFTSize(int size);
    void setHopSize(int hop);
    void setWindow(const QString& type);
    QVector<QVector<double>> forward(const QVector<double>& signal);
    QVector<double> inverse(const QVector<QVector<double>>& frames);
    QVector<double> phaseVocoder(const QVector<double>& signal,
                                  double stretchFactor);

    int fftSize() const { return m_fftSize; }
    int hopSize() const { return m_hopSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frames, int fftSize);

private:
    int m_fftSize = 2048;
    int m_hopSize = 512;
    QString m_windowType = "hann";
    QVector<double> m_window;

    void buildWindow();
    QVector<double> applyWindow(const QVector<double>& frame) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
