/**
 * @file Crossfader.h
 * @brief 交叉淡入淡出 — 线性/等功率/S曲线/多通道混合
 */
#pragma once
#include <QObject>
#include <QVector>
class Crossfader : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalCrossfades = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    enum CurveType { Linear, EqualPower, SCurve, Cosine };
    explicit Crossfader(QObject* parent = nullptr);
    void setCurveType(CurveType type);
    void setPosition(double pos);
    void setSampleRate(double rate);
    double processOne(double sampleA, double sampleB);
    QVector<double> process(const QVector<double>& a, const QVector<double>& b);
    QVector<double> processFadeOut(const QVector<double>& input, int fadeSamples);
    QVector<double> processFadeIn(const QVector<double>& input, int fadeSamples);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void crossfadeComplete(int samples);
private:
    double gainA(double pos) const;
    double gainB(double pos) const;
    CurveType m_curve = EqualPower; double m_position = 0.0; double m_sampleRate = 44100.0;
    Stats m_stats; double m_timeSum = 0.0;
};
