/**
 * @file FormantTracker.h
 * @brief 共振峰跟踪 — LPC分析/峰值跟踪/轨迹平滑/F1-F2映射
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class FormantTracker : public QObject {
    Q_OBJECT
public:
    struct Formant { double frequency = 0.0; double bandwidth = 0.0; double amplitude = 0.0; };
    struct Stats { int totalFrames = 0; int totalFormantsFound = 0; double avgProcessingTimeMs = 0.0; };
    explicit FormantTracker(QObject* parent = nullptr);
    void setSampleRate(double rate);
    void setLPCOrder(int order);
    void setMaxFormants(int max);
    QVector<Formant> analyzeFrame(const QVector<double>& frame);
    QVector<QVector<Formant>> analyze(const QVector<double>& audio, int frameSize, int hopSize);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void frameAnalyzed(int frameIndex, int numFormants);
private:
    QVector<double> lpcAnalysis(const QVector<double>& frame);
    QVector<Formant> rootsToFormants(const QVector<QPair<double,double>>& roots);
    double m_sampleRate = 16000.0; int m_lpcOrder = 12; int m_maxFormants = 5;
    Stats m_stats; double m_timeSum = 0.0;
};
