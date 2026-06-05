/**
 * @file DiscreteCosine4.h
 * @brief 离散余弦变换4 — MDCT+IMDCT+窗重叠
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class DiscreteCosine4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalSamplesProcessed = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DiscreteCosine4(QObject* parent = nullptr);

    void setSize(int n);
    QVector<double> dct(const QVector<double>& input) const;
    QVector<double> idct(const QVector<double>& coefficients) const;
    QVector<double> mdct(const QVector<double>& input);
    QVector<double> imdct(const QVector<double>& coefficients);
    QVector<double> mdctSequence(const QVector<double>& signal, int frameSize);

    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, const QString& type);

private:
    int m_n = 256;
    QVector<double> m_window;
    QVector<double> m_overlapBuffer;

    void buildWindow();
    void fft(QVector<double>& real, QVector<double>& imag, int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
