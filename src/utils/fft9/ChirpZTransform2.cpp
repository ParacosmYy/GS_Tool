/**
 * @file ChirpZTransform2.cpp
 * @brief Chirp Z变换实现 — 任意频率轴频谱分析
 */

#include "ChirpZTransform2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---------- 构造函数 ---------- */

ChirpZTransform2::ChirpZTransform2(QObject* parent)
    : QObject(parent)
{
}

/* ---------- 频率范围CZT ---------- */

ChirpZTransform2::CZTResult ChirpZTransform2::transform(
    const QVector<double>& input,
    double sampleRate,
    double freqStart,
    double freqEnd,
    int numOutput) const
{
    QElapsedTimer timer;
    timer.start();

    CZTResult result;
    int N = input.size();
    int M = (numOutput > 0) ? numOutput : N;

    if (N == 0 || M <= 0) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
        return result;
    }

    /* 转换为复数输入 */
    QVector<Complex> x;
    x.reserve(N);
    for (int i = 0; i < N; ++i) {
        x.append({input[i], 0.0});
    }

    /* 计算螺旋参数 */
    double wStart = 2.0 * M_PI * freqStart / sampleRate;
    double wEnd = 2.0 * M_PI * freqEnd / sampleRate;
    double dw = (wEnd - wStart) / M;

    Complex A = polar(1.0, wStart);
    Complex W = polar(1.0, -dw);

    /* 执行通用CZT */
    QVector<Complex> spectrum = transformGeneral(x, A, W, M);

    /* 计算频率轴 */
    result.frequencies.reserve(M);
    for (int k = 0; k < M; ++k) {
        double freq = freqStart + k * (freqEnd - freqStart) / M;
        result.frequencies.append(freq);
    }

    result.spectrum = spectrum;
    result.magnitudes = computeMagnitudes(spectrum);
    result.phases = computePhases(spectrum);
    result.inputSize = N;
    result.outputSize = M;

    m_stats.totalTransforms++;
    m_stats.totalInputSamples += N;
    m_stats.totalOutputBins += M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M, timer.elapsed());
    return result;
}

/* ---------- 通用CZT(Bluestein算法) ---------- */

QVector<ChirpZTransform2::Complex> ChirpZTransform2::transformGeneral(
    const QVector<Complex>& input,
    const Complex& A,
    const Complex& W,
    int M) const
{
    int N = input.size();
    int L = nextPow2(N + M - 1);

    /* Step 1: 计算 y_n = x_n * A^{-n} * W^{n^2/2} */
    QVector<Complex> y(L, {0.0, 0.0});
    Complex aInv = {A.first, -A.second}; /* A^{-1} = conj(A) 若|A|=1 */

    for (int n = 0; n < N; ++n) {
        Complex an = polar(1.0, -n * std::atan2(A.second, A.first));
        double nSqHalf = 0.5 * n * n;
        Complex wnSq = polar(1.0, -nSqHalf * std::atan2(W.second, W.first));
        y[n] = cmul(cmul(input[n], an), wnSq);
    }

    /* Step 2: 计算 v_k = W^{-k^2/2} */
    QVector<Complex> v(L, {0.0, 0.0});
    for (int k = 0; k < M; ++k) {
        double kSqHalf = 0.5 * k * k;
        v[k] = cconj(polar(1.0, -kSqHalf * std::atan2(W.second, W.first)));
    }

    /* 填充 v 的负索引部分(循环) */
    for (int k = 1; k < N; ++k) {
        double kSqHalf = 0.5 * k * k;
        v[L - k] = cconj(polar(1.0, -kSqHalf * std::atan2(W.second, W.first)));
    }

    /* Step 3: FFT卷积 Y * V */
    fft(y, false);
    fft(v, false);

    QVector<Complex> g(L);
    for (int i = 0; i < L; ++i) {
        g[i] = cmul(y[i], v[i]);
    }

    fft(g, true); /* 逆FFT */

    /* Step 4: 乘以 W^{k^2/2} */
    QVector<Complex> result;
    result.reserve(M);

    for (int k = 0; k < M; ++k) {
        double kSqHalf = 0.5 * k * k;
        Complex wkSq = polar(1.0, -kSqHalf * std::atan2(W.second, W.first));
        result.append(cmul(g[k], wkSq));
    }

    return result;
}

/* ---------- FFT(Cooley-Tukey) ---------- */

void ChirpZTransform2::fft(QVector<Complex>& data, bool inverse) const
{
    int n = data.size();
    if (n <= 1) return;

    /* 位反转置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        Complex wlen = {std::cos(angle), std::sin(angle)};

        for (int i = 0; i < n; i += len) {
            Complex w = {1.0, 0.0};
            for (int j = 0; j < len / 2; ++j) {
                Complex u = data[i + j];
                Complex v = cmul(w, data[i + j + len / 2]);
                data[i + j] = cadd(u, v);
                data[i + j + len / 2] = {u.first - v.first,
                                          u.second - v.second};
                w = cmul(w, wlen);
            }
        }
    }

    /* 逆变换归一化 */
    if (inverse) {
        for (int i = 0; i < n; ++i) {
            data[i].first /= n;
            data[i].second /= n;
        }
    }
}

/* ---------- 幅度谱 ---------- */

QVector<double> ChirpZTransform2::computeMagnitudes(
    const QVector<Complex>& spectrum) const
{
    QVector<double> mags;
    mags.reserve(spectrum.size());
    for (const auto& c : spectrum) {
        mags.append(std::sqrt(c.first * c.first + c.second * c.second));
    }
    return mags;
}

/* ---------- 相位谱 ---------- */

QVector<double> ChirpZTransform2::computePhases(
    const QVector<Complex>& spectrum) const
{
    QVector<double> phases;
    phases.reserve(spectrum.size());
    for (const auto& c : spectrum) {
        phases.append(std::atan2(c.second, c.first));
    }
    return phases;
}

/* ---------- 复数运算 ---------- */

ChirpZTransform2::Complex ChirpZTransform2::cmul(
    const Complex& a, const Complex& b) const
{
    return {a.first * b.first - a.second * b.second,
            a.first * b.second + a.second * b.first};
}

ChirpZTransform2::Complex ChirpZTransform2::cadd(
    const Complex& a, const Complex& b) const
{
    return {a.first + b.first, a.second + b.second};
}

ChirpZTransform2::Complex ChirpZTransform2::cconj(const Complex& a) const
{
    return {a.first, -a.second};
}

int ChirpZTransform2::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

ChirpZTransform2::Complex ChirpZTransform2::polar(
    double r, double theta) const
{
    return {r * std::cos(theta), r * std::sin(theta)};
}

/* ---------- 统计 ---------- */

ChirpZTransform2::Stats ChirpZTransform2::stats() const { return m_stats; }

void ChirpZTransform2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
