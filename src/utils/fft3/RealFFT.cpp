/**
 * @file RealFFT.cpp
 * @brief 实值快速傅里叶变换(RFFT)实现 — 针对实数输入优化
 */

#include "RealFFT.h"

#include <QElapsedTimer>
#include <cmath>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

RealFFT::RealFFT(QObject* parent)
    : QObject(parent)
{
}

RealFFT::~RealFFT() = default;

// ═══════════════════════════════════════════════════════════
// 变换
// ═══════════════════════════════════════════════════════════

QVector<RealFFT::Complex> RealFFT::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_stats.totalFFTs += 1;
        emit forwardCompleted(0);
        return {};
    }

    // 补零到2的幂
    const int N = nextPowerOf2(input.size());
    QVector<double> re(N, 0.0);
    QVector<double> im(N, 0.0);

    for (int i = 0; i < input.size(); ++i) {
        re[i] = input[i];
    }

    // 执行基2 FFT
    butterfly(re, im, false);

    // 利用实数信号的共轭对称性, 只返回N/2+1个频率点
    const int binCount = N / 2 + 1;
    QVector<Complex> spectrum;
    spectrum.reserve(binCount);

    for (int k = 0; k < binCount; ++k) {
        spectrum.append({re[k], im[k]});
    }

    m_stats.totalFFTs += 1;
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit forwardCompleted(binCount);
    return spectrum;
}

QVector<double> RealFFT::inverse(const QVector<Complex>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    if (spectrum.isEmpty()) {
        m_stats.totalIFFTs += 1;
        emit inverseCompleted(0);
        return {};
    }

    // 从N/2+1个频率点重建完整N点频谱(利用共轭对称性)
    const int binCount = spectrum.size();
    const int N = (binCount - 1) * 2;

    QVector<double> re(N, 0.0);
    QVector<double> im(N, 0.0);

    // 填充正频率部分
    for (int k = 0; k < binCount; ++k) {
        re[k] = spectrum[k].first;
        im[k] = spectrum[k].second;
    }

    // 填充负频率部分(共轭对称)
    for (int k = 1; k < binCount - 1; ++k) {
        re[N - k] = re[k];
        im[N - k] = -im[k];
    }

    // 执行IFFT
    butterfly(re, im, true);

    // 提取实部
    QVector<double> result;
    result.reserve(N);
    for (int i = 0; i < N; ++i) {
        result.append(re[i]);
    }

    m_stats.totalIFFTs += 1;
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit inverseCompleted(N);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 辅助
// ═══════════════════════════════════════════════════════════

QVector<double> RealFFT::magnitude(const QVector<Complex>& spectrum) const
{
    QVector<double> mag;
    mag.reserve(spectrum.size());
    for (const auto& c : spectrum) {
        mag.append(std::sqrt(c.first * c.first + c.second * c.second));
    }
    return mag;
}

QVector<double> RealFFT::powerSpectrum(const QVector<Complex>& spectrum) const
{
    QVector<double> power;
    power.reserve(spectrum.size());
    for (const auto& c : spectrum) {
        power.append(c.first * c.first + c.second * c.second);
    }
    return power;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

RealFFT::Stats RealFFT::stats() const
{
    return m_stats;
}

void RealFFT::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

void RealFFT::butterfly(QVector<double>& re, QVector<double>& im,
                        bool inverse) const
{
    const int N = re.size();
    if (N <= 1) return;

    // 位反转重排
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Cooley-Tukey蝶形运算
    const double sign = inverse ? 1.0 : -1.0;

    for (int len = 2; len <= N; len <<= 1) {
        const double angle = sign * 2.0 * M_PI / static_cast<double>(len);
        const double wRe = std::cos(angle);
        const double wIm = std::sin(angle);

        for (int i = 0; i < N; i += len) {
            double curRe = 1.0;
            double curIm = 0.0;

            for (int j = 0; j < len / 2; ++j) {
                const int u = i + j;
                const int v = i + j + len / 2;

                const double tRe = curRe * re[v] - curIm * im[v];
                const double tIm = curRe * im[v] + curIm * re[v];

                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] = re[u] + tRe;
                im[u] = im[u] + tIm;

                const double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    // IFFT需要除以N
    if (inverse) {
        const double invN = 1.0 / static_cast<double>(N);
        for (int i = 0; i < N; ++i) {
            re[i] *= invN;
            im[i] *= invN;
        }
    }
}

int RealFFT::nextPowerOf2(int n) const
{
    if (n <= 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

void RealFFT::bitReverse(QVector<double>& data, int n) const
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void RealFFT::updateAvgTime(double elapsedMs) const
{
    const auto total = m_stats.totalFFTs + m_stats.totalIFFTs;
    if (total <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(total - 1) / static_cast<double>(total) +
            elapsedMs / static_cast<double>(total);
    }
}
