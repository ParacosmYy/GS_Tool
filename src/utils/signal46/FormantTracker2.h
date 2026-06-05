/**
 * @file FormantTracker2.h
 * @brief 共振峰跟踪2 — LPC+卡尔曼平滑
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class FormantTracker2 : public QObject
{
    Q_OBJECT

public:
    struct Formant {
        double frequency;
        double bandwidth;
        double amplitude;
    };

    struct Stats {
        int totalTracks = 0;
        int totalFrames = 0;
        int numFormants = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FormantTracker2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setNumFormants(int n);
    void setLPCOrder(int order);
    QVector<Formant> track(const QVector<double>& frame);
    QVector<QVector<Formant>> trackSequence(const QVector<double>& signal,
                                             int frameSize, int hopSize);

    int numFormants() const { return m_numFormants; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trackingCompleted(int numFormants, double fundamentalFreq);

private:
    double m_sampleRate = 44100.0;
    int m_numFormants = 4;
    int m_lpcOrder = 12;
    QVector<Formant> m_prevFormants;

    QVector<double> lpcAnalysis(const QVector<double>& frame) const;
    QVector<Formant> extractFormants(const QVector<double>& lpcCoeffs) const;
    QVector<Formant> kalmanSmooth(const QVector<Formant>& current,
                                   const QVector<Formant>& previous) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
