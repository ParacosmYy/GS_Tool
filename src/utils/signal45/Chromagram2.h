/**
 * @file Chromagram2.h
 * @brief 色度图2 — 和弦检测+调性估计
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class Chromagram2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalComputations = 0;
        int totalFrames = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Chromagram2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setFFTSize(int fftSize);
    void setHopSize(int hopSize);
    QVector<double> compute(const QVector<double>& frame);
    QVector<QVector<double>> computeSequence(const QVector<double>& signal);
    QString detectChord(const QVector<double>& chroma) const;
    QString detectKey(const QVector<QVector<double>>& chromaSequence) const;

    int numNotes() const { return 12; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void chordDetected(const QString& chord, double confidence);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 8192;
    int m_hopSize = 4096;
    QVector<QVector<double>> m_noteFilters;
    bool m_initialized = false;

    void buildNoteFilters();
    QVector<double> chordTemplate(const QString& chord) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
