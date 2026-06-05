/**
 * @file HilbertTransform.cpp
 * @brief Hilbert变换实现 — 频域方法
 */

#include "utils/hilbert/HilbertTransform.h"

#include <QElapsedTimer>
#include <cmath>

HilbertTransformer::HilbertTransformer(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

int HilbertTransformer::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

void HilbertTransformer::fft(QVector<std::complex<double>>& data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;

    /* 位逆序排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        std::complex<double> wn(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                auto u = data[i + j];
                auto v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wn;
            }
        }
    }

    if (inverse) {
        for (auto& v : data) v /= n;
    }
}

QVector<double> HilbertTransformer::transform(const QVector<double>& signal)
{
    auto analytic = analyticSignal(signal);
    QVector<double> result;
    result.reserve(analytic.size());
    for (const auto& c : analytic) {
        double imag = c.imag();
        /* 保留虚部(Hilbert变换结果) */
        if (static_cast<int>(result.size()) >= signal.size()) break;
        result.append(imag);
    }
    /* 截取到原始长度 */
    if (result.size() > signal.size())
        result.resize(signal.size());
    return result;
}

QVector<std::complex<double>> HilbertTransformer::analyticSignal(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = nextPow2(signal.size());

    /* 零填充到2的幂 */
    QVector<std::complex<double>> X(n);
    for (int i = 0; i < signal.size(); ++i)
        X[i] = std::complex<double>(signal[i], 0.0);

    /* FFT */
    fft(X, false);

    /* 构造解析信号: 正频率×2, DC和Nyquist×1, 负频率×0 */
    for (int i = 1; i < n / 2; ++i)
        X[i] *= 2.0;
    for (int i = n / 2 + 1; i < n; ++i)
        X[i] = std::complex<double>(0.0, 0.0);

    /* IFFT */
    fft(X, true);

    /* 截取到原始长度 */
    QVector<std::complex<double>> result(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        result[i] = X[i];

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(signal.size());
    return result;
}

QVector<double> HilbertTransformer::instantaneousFrequency(
    const QVector<double>& signal, double sampleRate)
{
    auto analytic = analyticSignal(signal);
    int n = analytic.size();
    if (n < 2) return {};

    QVector<double> freq(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        double phase1 = std::arg(analytic[i]);
        double phase2 = std::arg(analytic[i + 1]);
        double dPhase = phase2 - phase1;
        /* 相位展开 */
        while (dPhase > M_PI) dPhase -= 2.0 * M_PI;
        while (dPhase < -M_PI) dPhase += 2.0 * M_PI;
        freq[i] = dPhase * sampleRate / (2.0 * M_PI);
    }
    return freq;
}

QVector<double> HilbertTransformer::envelope(const QVector<double>& signal)
{
    auto analytic = analyticSignal(signal);
    QVector<double> env(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        env[i] = std::abs(analytic[i]);
    return env;
}

void HilbertTransformer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
