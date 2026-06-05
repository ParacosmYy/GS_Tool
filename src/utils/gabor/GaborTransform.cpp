/**
 * @file GaborTransform.cpp
 * @brief Gabor变换实现
 */

#include "utils/gabor/GaborTransform.h"

#include <QtMath>
#include <QElapsedTimer>

GaborTransform::GaborTransform(QObject* parent)
    : QObject(parent), m_windowSize(64), m_sigma(10.0),
      m_numFreqs(32), m_timeSum(0.0) {}

void GaborTransform::setParameters(int windowSize, double sigma, int numFreqs)
{
    m_windowSize = qMax(4, windowSize);
    m_sigma = qMax(1.0, sigma);
    m_numFreqs = qMax(1, numFreqs);
}

QVector<QVector<double>> GaborTransform::transform(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    int half = m_windowSize / 2;
    int numFrames = qMax(1, N - m_windowSize + 1);

    QVector<QVector<double>> result(m_numFreqs, QVector<double>(numFrames, 0.0));

    for (int t = 0; t < numFrames; ++t) {
        for (int f = 0; f < m_numFreqs; ++f) {
            double freq = static_cast<double>(f) / m_numFreqs;
            double re = 0.0, im = 0.0;

            for (int n = 0; n < m_windowSize; ++n) {
                int idx = t + n;
                if (idx >= N) break;

                double center = half;
                double gauss = qExp(-(n - center) * (n - center) / (2.0 * m_sigma * m_sigma));
                double angle = 2.0 * M_PI * freq * n;
                re += data[idx] * gauss * qCos(angle);
                im += data[idx] * gauss * qSin(angle);
            }
            result[f][t] = qSqrt(re * re + im * im);
        }
    }

    m_stats.totalTransforms++;
    m_stats.totalCoefficientsComputed += numFrames * m_numFreqs;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numFrames, m_numFreqs);
    return result;
}

void GaborTransform::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
