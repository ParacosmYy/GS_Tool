/**
 * @file HilbertTransform2.h
 * @brief 希尔伯特变换增强 — FIR设计/频域解析/瞬时频率/包络提取
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class HilbertTransform2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit HilbertTransform2(QObject* parent = nullptr);
    void setFilterLength(int length);
    QVector<double> analyticalSignal(const QVector<double>& input);
    QVector<double> envelope(const QVector<double>& input);
    QVector<double> instantaneousPhase(const QVector<double>& input);
    QVector<double> instantaneousFrequency(const QVector<double>& input, double sampleRate);
    QVector<double> designHilbertFIR(int length) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int samples);
private:
    void fft(QVector<double>& real, QVector<double>& imag);
    int m_filterLength = 64;
    Stats m_stats; double m_timeSum = 0.0;
};
