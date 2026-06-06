/**
 * @file ModifiedDCT.cpp
 * @brief ModifiedDCT 实现
 *
 * 实现MDCT/IMDCT变换：正弦窗加窗、时域混叠消除、
 * 50%重叠相加完美重建。
 */

#include "utils/fft166/ModifiedDCT.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

ModifiedDCT::ModifiedDCT(QObject* parent)
    : QObject(parent)
{
}

ModifiedDCT::~ModifiedDCT() = default;

void ModifiedDCT::setBlockSize(int N)
{
    m_blockSize = qMax(4, (N / 2) * 2); /* Ensure even */
}

QVector<double> ModifiedDCT::sineWindow(int len) const
{
    QVector<double> w(len);
    for (int n = 0; n < len; ++n) {
        w[n] = qSin(M_PI * (n + 0.5) / len);
    }
    return w;
}

void ModifiedDCT::computeMDCT(const QVector<double>& windowed, QVector<double>& out) const
{
    /* MDCT: X[k] = sum_{n=0}^{2N-1} x[n] * cos(pi*(2n+1+N)*(2k+1)/(4N)) */
    const int N = m_blockSize;
    const int len2N = 2 * N;
    out.resize(N);

    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < len2N; ++n) {
            double angle = M_PI * (2 * n + 1 + N) * (2 * k + 1) / (4.0 * N);
            sum += windowed[n] * qCos(angle);
        }
        out[k] = sum;
    }
}

void ModifiedDCT::computeIMDCT(const QVector<double>& coeffs, QVector<double>& out) const
{
    /* IMDCT: y[n] = sum_{k=0}^{N-1} X[k] * cos(pi*(2n+1+N)*(2k+1)/(4N)) */
    const int N = m_blockSize;
    const int len2N = 2 * N;
    out.resize(len2N);

    for (int n = 0; n < len2N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double angle = M_PI * (2 * n + 1 + N) * (2 * k + 1) / (4.0 * N);
            sum += coeffs[k] * qCos(angle);
        }
        out[n] = sum / N;
    }
}

QVector<double> ModifiedDCT::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_blockSize;
    const int len2N = 2 * N;

    if (input.size() < len2N) return QVector<double>();

    /* Apply sine window */
    QVector<double> window = sineWindow(len2N);
    QVector<double> windowed(len2N);
    for (int i = 0; i < len2N; ++i) {
        windowed[i] = input[i] * window[i];
    }

    /* Compute MDCT */
    QVector<double> result;
    computeMDCT(windowed, result);

    m_stats.totalForward++;
    /* Find peak */
    int peak = 0;
    double maxVal = qAbs(result[0]);
    for (int i = 1; i < result.size(); ++i) {
        if (qAbs(result[i]) > maxVal) { maxVal = qAbs(result[i]); peak = i; }
    }
    m_stats.lastPeakBin = peak;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit forwardCompleted(N);
    return result;
}

QVector<double> ModifiedDCT::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    const int N = m_blockSize;
    const int len2N = 2 * N;

    if (coeffs.size() < N) return QVector<double>();

    /* Compute IMDCT */
    QVector<double> raw;
    computeIMDCT(coeffs, raw);

    /* Apply sine window */
    QVector<double> window = sineWindow(len2N);
    QVector<double> result(len2N);
    for (int i = 0; i < len2N; ++i) {
        result[i] = raw[i] * window[i];
    }

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inverseCompleted(N);
    return result;
}

QVector<double> ModifiedDCT::overlapAdd(const QVector<QVector<double>>& blocks)
{
    if (blocks.isEmpty()) return QVector<double>();

    const int N = m_blockSize;
    const int len2N = 2 * N;
    int totalLen = N * (blocks.size() + 1);

    QVector<double> output(totalLen, 0.0);

    for (int b = 0; b < blocks.size(); ++b) {
        QVector<double> reconstructed = inverse(blocks[b]);
        int offset = b * N;
        for (int i = 0; i < len2N && offset + i < totalLen; ++i) {
            output[offset + i] += reconstructed[i];
        }
    }

    return output;
}

void ModifiedDCT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
