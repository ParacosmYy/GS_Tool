#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MFCC2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit MFCC2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setNumCoeffs(int n);
    void setNumFilters(int n);
    QVector<double> compute(const QVector<double>& frame);
    QVector<double> melSpectrum() const { return m_melSpec; }
    int numCoeffs() const { return m_numCoeffs; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int coeffs, double energy);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 512; int m_numCoeffs = 13; int m_numFilters = 26;
    QVector<double> m_melSpec;
    double hzToMel(double hz) const;
    double melToHz(double mel) const;
    QVector<double> melFilterBank(const QVector<double>& spectrum);
    QVector<double> dctII(const QVector<double>& input);
    Stats m_stats; double m_timeSum = 0.0;
};
