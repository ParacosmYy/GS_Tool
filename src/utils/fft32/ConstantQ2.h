/**
 * @file ConstantQ2.h
 * @brief 常数Q变换增强 — 对数频率/多分辨率/CQT逆变换
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class ConstantQ2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalBinsComputed = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConstantQ2(QObject* parent = nullptr);
    void setSampleRate(double rate);
    void setFrequencyRange(double minFreq, double maxFreq);
    void setBinsPerOctave(int bins);
    void setThreshold(double thresh);
    QVector<double> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<double>& cqtCoeffs);
    QVector<double> frequencies() const;
    int totalBins() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int bins);
private:
    double m_sampleRate = 44100.0; double m_minFreq = 27.5; double m_maxFreq = 4186.0;
    int m_binsPerOctave = 12; double m_thresh = 0.01; int m_totalBins = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
