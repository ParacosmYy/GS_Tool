/**
 * @file StereoProcessor3.h
 * @brief 立体声处理器3 — MS编解码+声场旋转
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class StereoProcessor3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StereoProcessor3(QObject* parent = nullptr);

    void setWidth(double width);
    void setRotation(double degrees);
    void setBalance(double balance);
    void setMonoBelow(double freqHz);
    QPair<QVector<double>,QVector<double>> process(
        const QVector<double>& left, const QVector<double>& right);
    QPair<QVector<double>,QVector<double>> toMS(
        const QVector<double>& left, const QVector<double>& right) const;
    QPair<QVector<double>,QVector<double>> toLR(
        const QVector<double>& mid, const QVector<double>& side) const;

    double width() const { return m_width; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double correlation);

private:
    double m_width = 1.0;
    double m_rotation = 0.0;
    double m_balance = 0.0;
    double m_monoBelow = 80.0;
    double m_sampleRate = 44100.0;
    QVector<double> m_lpfState;

    QVector<double> lowpass(const QVector<double>& input);

    Stats m_stats;
    double m_timeSum = 0.0;
};
