/**
 * @file EnvelopeDetector.cpp
 * @brief 包络检测器实现 — Hilbert/峰值/RMS
 */

#include "utils/envelope/EnvelopeDetector.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

EnvelopeDetector::EnvelopeDetector(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> EnvelopeDetector::detect(const QVector<double>& data,
                                          Method method) const
{
    QVector<double> result;
    if (data.isEmpty()) return result;

    QElapsedTimer timer;
    timer.start();

    switch (method) {
    case Method::Hilbert: {
        QVector<double> imag = hilbertTransform(data);
        result.resize(data.size());
        for (int i = 0; i < data.size(); ++i) {
            result[i] = qSqrt(data[i] * data[i] + imag[i] * imag[i]);
        }
        break;
    }
    case Method::PeakHold:
        result = peakHoldEnvelope(data, 32, 0.95);
        break;
    case Method::LogDetect: {
        QVector<double> hilbertEnv;
        QVector<double> imag = hilbertTransform(data);
        hilbertEnv.resize(data.size());
        for (int i = 0; i < data.size(); ++i) {
            hilbertEnv[i] = qSqrt(data[i] * data[i] + imag[i] * imag[i]);
        }
        result.resize(data.size());
        for (int i = 0; i < data.size(); ++i) {
            result[i] = 20.0 * std::log10(qMax(hilbertEnv[i], 1e-15));
        }
        break;
    }
    case Method::Squelch:
        result = rmsEnvelope(data, 64);
        break;
    }

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(data.size(), method);
    return result;
}

QVector<double> EnvelopeDetector::hilbertTransform(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    if (n == 0) return result;

    /* 简化Hilbert: 频域乘以-j (正频率) / +j (负频率) */
    /* 使用直接卷积近似: h[n] = 2/(π*n) for odd n */
    int halfLen = qMin(128, n / 2);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = -halfLen; k <= halfLen; ++k) {
            if (k == 0 || (k % 2 == 0)) continue;
            int idx = i - k;
            if (idx < 0 || idx >= n) continue;
            sum += data[idx] * (2.0 / (M_PI * k));
        }
        result[i] = sum;
    }
    return result;
}

QVector<double> EnvelopeDetector::peakHoldEnvelope(const QVector<double>& data,
                                                    int holdTime, double decay) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    if (n == 0) return result;

    double peak = 0.0;
    int holdCounter = 0;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(data[i]);
        if (absVal >= peak) {
            peak = absVal;
            holdCounter = holdTime;
        } else if (holdCounter > 0) {
            --holdCounter;
        } else {
            peak *= decay;
        }
        result[i] = peak;
    }
    return result;
}

QVector<double> EnvelopeDetector::rmsEnvelope(const QVector<double>& data,
                                               int windowSize) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    if (n == 0 || windowSize < 1) return result;

    int half = windowSize / 2;
    for (int i = 0; i < n; ++i) {
        double sumSq = 0.0;
        int count = 0;
        for (int j = qMax(0, i - half); j <= qMin(n - 1, i + half); ++j) {
            sumSq += data[j] * data[j];
            ++count;
        }
        result[i] = (count > 0) ? qSqrt(sumSq / count) : 0.0;
    }
    return result;
}

void EnvelopeDetector::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
