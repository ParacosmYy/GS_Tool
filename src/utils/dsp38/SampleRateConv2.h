#pragma once
#include <QObject>
#include <QVector>
class SampleRateConv2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalConversions = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SampleRateConv2(QObject* parent = nullptr);
    void setSourceRate(double rate); void setTargetRate(double rate);
    void setQuality(int taps);
    QVector<double> convert(const QVector<double>& input);
    double ratio() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void conversionComplete(int inSamples, int outSamples);
private:
    int gcd(int a, int b) const;
    void reduceFraction(int numerator, int denominator, int& outL, int& outM) const;
    QVector<double> designKaiserFilter(double cutoff, int length, double beta = 8.0) const;
    double besselI0(double x) const;
    double m_srcRate = 44100.0; double m_dstRate = 48000.0; int m_taps = 64;
    Stats m_stats; double m_timeSum = 0.0;
};
