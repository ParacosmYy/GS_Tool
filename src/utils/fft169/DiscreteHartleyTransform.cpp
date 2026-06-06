/**
 * @file DiscreteHartleyTransform.cpp
 * @brief DiscreteHartleyTransform 实现
 *
 * 实现DHT：Cas2Cas蝶形运算、自逆性质、DHT域卷积/相关。
 */

#include "utils/fft169/DiscreteHartleyTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

DiscreteHartleyTransform::DiscreteHartleyTransform(QObject *parent)
    : QObject(parent)
{
}

DiscreteHartleyTransform::~DiscreteHartleyTransform() = default;

double DiscreteHartleyTransform::cas(double x)
{
    return qCos(x) + qSin(x);
}

void DiscreteHartleyTransform::dhtCore(QVector<double>& data) const
{
    int n = data.size();

    /* Bit-reversal permutation */
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int x = i;
        for (int b = 0; b < log2n; ++b) {
            rev = (rev << 1) | (x & 1);
            x >>= 1;
        }
        if (rev > i) std::swap(data[i], data[rev]);
    }

    /* Cas2Cas butterfly (Bracewell's algorithm) */
    for (int len = 2; len <= n; len <<= 1) {
        int halfLen = len / 2;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                double a = data[i + j];
                double b = data[i + j + halfLen];

                /* Cas2Cas: cas(j*θ) and cas((halfLen-j)*θ) decomposition */
                double angle = 2.0 * M_PI * j / len;
                double cosVal = qCos(angle);
                double sinVal = qSin(angle);

                /* H[k] and H[k+N/2] combine using cas symmetry */
                data[i + j] = a + b * cosVal + (a - b) * sinVal;
                data[i + j + halfLen] = a - b * cosVal + (a + b) * sinVal;
            }
        }
    }
}

QVector<double> DiscreteHartleyTransform::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    /* Pad to power of 2 */
    int n = input.size();
    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<double> data(sz, 0.0);
    for (int i = 0; i < n; ++i) data[i] = input[i];

    dhtCore(data);
    data.resize(n);

    m_stats.totalForward++;
    m_stats.lastTransformSize = sz;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit forwardCompleted(sz);
    return data;
}

QVector<double> DiscreteHartleyTransform::inverse(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    /* DHT is self-inverse: apply DHT again and divide by N */
    int n = coeffs.size();
    int sz = 1;
    while (sz < n) sz <<= 1;

    QVector<double> data(sz, 0.0);
    for (int i = 0; i < n; ++i) data[i] = coeffs[i];

    dhtCore(data);

    for (int i = 0; i < n; ++i)
        data[i] /= static_cast<double>(sz);

    data.resize(n);

    m_stats.totalInverse++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalForward + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inverseCompleted(sz);
    return data;
}

QVector<double> DiscreteHartleyTransform::cyclicConvolution(
    const QVector<double>& a, const QVector<double>& b)
{
    int na = a.size(), nb = b.size();
    int len = qMax(na, nb);
    int sz = 1;
    while (sz < len) sz <<= 1;

    QVector<double> pa(sz, 0.0), pb(sz, 0.0);
    for (int i = 0; i < na; ++i) pa[i] = a[i];
    for (int i = 0; i < nb; ++i) pb[i] = b[i];

    dhtCore(pa);
    dhtCore(pb);

    /* DHT convolution: result[k] = 0.5*(Ha[k]*(Hb[k]+Hb[N-k]) + Ha[N-k]*(Hb[k]-Hb[N-k])) */
    QVector<double> result(sz);
    for (int k = 0; k < sz; ++k) {
        int km = (k == 0) ? 0 : sz - k;
        double hk = pa[k];
        double hkm = pa[km];
        double gk = pb[k];
        double gkm = pb[km];
        result[k] = 0.5 * (hk * (gk + gkm) + hkm * (gk - gkm));
    }

    dhtCore(result);

    int resultLen = qMin(na + nb - 1, sz);
    result.resize(qMax(1, resultLen));
    return result;
}

QVector<double> DiscreteHartleyTransform::crossCorrelation(
    const QVector<double>& a, const QVector<double>& b)
{
    int na = a.size(), nb = b.size();
    int len = qMax(na, nb);
    int sz = 1;
    while (sz < len) sz <<= 1;

    QVector<double> pa(sz, 0.0), pb(sz, 0.0);
    for (int i = 0; i < na; ++i) pa[i] = a[i];
    for (int i = 0; i < nb; ++i) pb[i] = b[i];

    dhtCore(pa);
    dhtCore(pb);

    /* Cross-correlation via DHT */
    QVector<double> result(sz);
    for (int k = 0; k < sz; ++k) {
        int km = (k == 0) ? 0 : sz - k;
        result[k] = 0.5 * (pa[k] * (pb[k] + pb[km]) - pa[km] * (pb[k] - pb[km]));
    }

    dhtCore(result);

    int resultLen = qMin(na + nb - 1, sz);
    result.resize(qMax(1, resultLen));
    return result;
}

void DiscreteHartleyTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
