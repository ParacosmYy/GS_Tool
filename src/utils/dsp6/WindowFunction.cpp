/**
 * @file WindowFunction.cpp
 * @brief DSP窗函数实现 — Kaiser/Chebyshev/Gaussian/Dolph-Chebyshev
 */

#include "utils/dsp6/WindowFunction.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

WindowFunction::WindowFunction(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> WindowFunction::kaiser(int N, double beta)
{
    QElapsedTimer timer;
    timer.start();

    if (N <= 0) return {};

    QVector<double> w(N);
    double denom = besselI0(beta);
    double halfN = (N - 1) / 2.0;

    for (int n = 0; n < N; ++n) {
        double ratio = (n - halfN) / halfN;
        double arg = beta * qSqrt(qMax(0.0, 1.0 - ratio * ratio));
        w[n] = besselI0(arg) / denom;
    }

    ++m_stats.totalWindowsGenerated;
    m_stats.totalPointsComputed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWindowsGenerated;

    emit windowGenerated(Type::Kaiser, N);
    return w;
}

QVector<double> WindowFunction::chebyshev(int N, double attenuation)
{
    QElapsedTimer timer;
    timer.start();

    if (N <= 0) return {};

    QVector<double> w(N);

    if (N == 1) {
        w[0] = 1.0;
        return w;
    }

    /* 计算主瓣参数 */
    double M = N - 1;
    double beta = qPow(10.0, attenuation / 20.0);

    /* Chebyshev窗通过频域采样IDFT得到 */
    double x0 = qCosh((1.0 / M) * qAcosh(beta));

    QVector<double> freqResp(N);
    freqResp[0] = beta;

    for (int n = 1; n <= N / 2; ++n) {
        double sum = 0.0;
        for (int k = 1; k <= M; ++k) {
            double xn = qCos(2.0 * M_PI * k * n / N);
            sum += chebyshevPoly(static_cast<int>(M), xn);
        }
        freqResp[n] = freqResp[N - n] =
            qMax(1.0, qAbs(chebyshevPoly(static_cast<int>(M),
                x0 * qCos(M_PI * n / N))));
    }

    /* IDFT得到时域窗函数 */
    for (int n = 0; n < N; ++n) {
        double sum = freqResp[0];
        for (int k = 1; k < N; ++k) {
            sum += freqResp[k] * qCos(2.0 * M_PI * k * n / N);
        }
        w[n] = sum / N;
    }

    /* 归一化到峰值=1 */
    double maxVal = *std::max_element(w.begin(), w.end());
    if (maxVal > 0) {
        for (auto& v : w) v /= maxVal;
    }

    ++m_stats.totalWindowsGenerated;
    m_stats.totalPointsComputed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWindowsGenerated;

    emit windowGenerated(Type::Chebyshev, N);
    return w;
}

QVector<double> WindowFunction::gaussian(int N, double sigma)
{
    QElapsedTimer timer;
    timer.start();

    if (N <= 0) return {};

    QVector<double> w(N);
    double halfN = (N - 1) / 2.0;
    double denom = 2.0 * sigma * sigma * (N / 2.0) * (N / 2.0);

    for (int n = 0; n < N; ++n) {
        double diff = (n - halfN) / halfN;
        w[n] = qExp(-(diff * diff) / (2.0 * sigma * sigma));
    }

    ++m_stats.totalWindowsGenerated;
    m_stats.totalPointsComputed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWindowsGenerated;

    emit windowGenerated(Type::Gaussian, N);
    return w;
}

QVector<double> WindowFunction::dolphChebyshev(int N, double attenuation)
{
    QElapsedTimer timer;
    timer.start();

    if (N <= 0) return {};

    /* Dolph-Chebyshev实际上是Chebyshev窗的最优形式 */
    QVector<double> w(N);

    if (N == 1) { w[0] = 1.0; return w; }

    double M = N - 1;
    double beta = qPow(10.0, attenuation / 20.0);
    double ratio = beta + qSqrt(beta * beta - 1.0);
    double x0 = 0.5 * (qPow(ratio, 1.0 / M) + qPow(ratio, -1.0 / M));

    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k <= static_cast<int>(M); ++k) {
            double x = x0 * qCos(M_PI * (k + 0.5) / (M + 1));
            double Tn = chebyshevPoly(k, x);
            sum += Tn * qCos(2.0 * M_PI * n * (k + 0.5) / (M + 1));
        }
        w[n] = sum;
    }

    /* 归一化 */
    double maxVal = *std::max_element(w.begin(), w.end());
    if (maxVal > 0) {
        for (auto& v : w) v /= maxVal;
    }

    ++m_stats.totalWindowsGenerated;
    m_stats.totalPointsComputed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalWindowsGenerated;

    emit windowGenerated(Type::DolphChebyshev, N);
    return w;
}

QVector<double> WindowFunction::generate(Type type, int N, double param)
{
    switch (type) {
    case Type::Kaiser:         return kaiser(N, (param > 0) ? param : 8.0);
    case Type::Chebyshev:      return chebyshev(N, (param > 0) ? param : 60.0);
    case Type::Gaussian:       return gaussian(N, (param > 0) ? param : 0.4);
    case Type::DolphChebyshev: return dolphChebyshev(N, (param > 0) ? param : 60.0);
    case Type::Hamming: {
        QVector<double> w(N);
        for (int n = 0; n < N; ++n)
            w[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
        return w;
    }
    case Type::Blackman: {
        QVector<double> w(N);
        for (int n = 0; n < N; ++n)
            w[n] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (N - 1))
                      + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
        return w;
    }
    case Type::FlatTop: {
        QVector<double> w(N);
        for (int n = 0; n < N; ++n) {
            double t = 2.0 * M_PI * n / (N - 1);
            w[n] = 0.21557895 - 0.41663158 * qCos(t)
                   + 0.277263158 * qCos(2 * t)
                   - 0.083578947 * qCos(3 * t)
                   + 0.006947368 * qCos(4 * t);
        }
        return w;
    }
    }
    return QVector<double>(N, 1.0);
}

WindowFunction::WindowProperties WindowFunction::computeProperties(
    const QVector<double>& window)
{
    QElapsedTimer timer;
    timer.start();

    WindowProperties props;
    int N = window.size();
    if (N == 0) return props;

    /* 相干增益 */
    double sum = 0.0;
    for (double v : window) sum += v;
    props.coherentGain = sum / N;

    /* 等效噪声带宽 */
    double sumSq = 0.0;
    for (double v : window) sumSq += v * v;
    props.equivalentNoiseBW = N * sumSq / (sum * sum);

    /* 扇形损耗 */
    double correctedSum = 0.0;
    for (int n = 0; n < N; ++n) {
        correctedSum += window[n] * qCos(M_PI * (n - (N - 1) / 2.0) / N);
    }
    props.scallopingLoss = -20.0 * qLog10(qAbs(correctedSum / sum));

    /* 最差处理增益 */
    props.worstCaseProcessGain = 10.0 * qLog10(sumSq / (N * N));

    /* 3dB主瓣宽度 — 通过DFT分析 */
    auto spectrum = windowSpectrum(window, qMax(1024, N * 4));
    double peakDb = -300.0;
    for (double m : spectrum) {
        double db = 20.0 * qLog10(qMax(1e-20, m));
        if (db > peakDb) peakDb = db;
    }

    /* 找到-3dB点 */
    int left = -1, right = -1;
    double threshold = peakDb - 3.0;
    int peakIdx = 0;
    double peakVal = 0.0;
    for (int i = 0; i < spectrum.size() / 2; ++i) {
        if (spectrum[i] > peakVal) { peakVal = spectrum[i]; peakIdx = i; }
    }
    for (int i = peakIdx; i >= 0; --i) {
        if (20.0 * qLog10(qMax(1e-20, spectrum[i])) < threshold)
            { left = i; break; }
    }
    for (int i = peakIdx; i < spectrum.size() / 2; ++i) {
        if (20.0 * qLog10(qMax(1e-20, spectrum[i])) < threshold)
            { right = i; break; }
    }
    if (left >= 0 && right >= 0) {
        props.mainLobeWidth3dB = static_cast<double>(right - left) /
                                  spectrum.size();
    }

    /* 旁瓣电平(跳过主瓣区域) */
    int mainLobeEnd = qMax(peakIdx * 2, 3);
    double maxSideLobe = 0.0;
    for (int i = mainLobeEnd; i < spectrum.size() / 2; ++i) {
        if (spectrum[i] > maxSideLobe) maxSideLobe = spectrum[i];
    }
    props.sideLobeLevel = (maxSideLobe > 0)
        ? 20.0 * qLog10(maxSideLobe / peakVal) : -300.0;

    ++m_stats.totalPropertiesComputed;
    m_timeSum += timer.elapsed();
    if (m_stats.totalWindowsGenerated + m_stats.totalPropertiesComputed > 0)
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalWindowsGenerated + m_stats.totalPropertiesComputed);

    return props;
}

QVector<double> WindowFunction::apply(const QVector<double>& data,
                                       const QVector<double>& window)
{
    int N = qMin(data.size(), window.size());
    QVector<double> result(N);
    for (int i = 0; i < N; ++i) {
        result[i] = data[i] * window[i];
    }
    return result;
}

WindowFunction::Stats WindowFunction::stats() const
{
    return m_stats;
}

void WindowFunction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

double WindowFunction::besselI0(double x) const
{
    /* 零阶修正Bessel函数 — 级数展开 */
    double sum = 1.0;
    double term = 1.0;
    double halfX = x / 2.0;
    for (int k = 1; k <= 50; ++k) {
        term *= (halfX / k) * (halfX / k);
        sum += term;
        if (term < 1e-20 * sum) break;
    }
    return sum;
}

double WindowFunction::chebyshevPoly(int n, double x) const
{
    if (n == 0) return 1.0;
    if (n == 1) return x;

    double prev2 = 1.0, prev1 = x, result = 0.0;
    for (int i = 2; i <= n; ++i) {
        result = 2.0 * x * prev1 - prev2;
        prev2 = prev1;
        prev1 = result;
    }
    return result;
}

QVector<double> WindowFunction::windowSpectrum(
    const QVector<double>& window, int fftSize)
{
    /* 简化DFT幅度谱(仅计算正频率部分) */
    int N = window.size();
    int halfSize = fftSize / 2;
    QVector<double> magnitude(halfSize);

    for (int k = 0; k < halfSize; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / fftSize;
            re += window[n] * qCos(angle);
            im += window[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }
    return magnitude;
}
