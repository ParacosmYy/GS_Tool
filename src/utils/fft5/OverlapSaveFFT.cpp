/**
 * @file OverlapSaveFFT.cpp
 * @brief Overlap-Save快速卷积实现 — 分块FFT线性卷积
 */

#include "OverlapSaveFFT.h"

#include <QElapsedTimer>
#include <complex>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

OverlapSaveFFT::OverlapSaveFFT(QObject* parent)
    : QObject(parent)
{
}

OverlapSaveFFT::~OverlapSaveFFT() = default;

// ═══════════════════════════════════════════════════════════
// 核心接口
// ═══════════════════════════════════════════════════════════

QVector<double> OverlapSaveFFT::process(const QVector<double>& signal,
                                         const QVector<double>& kernel)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || kernel.isEmpty()) {
        return {};
    }

    const int M = kernel.size();                /* 卷积核长度 */
    const int N = m_blockSize;                  /* FFT大小 */
    const int L = N - M + 1;                    /* 每块有效输出长度 */

    /* 将卷积核补零到N点 */
    std::vector<std::complex<double>> H(N);
    for (int i = 0; i < M; ++i) {
        H[i] = kernel[i];
    }
    /* H剩余位置已经为零 */

    /* 对卷积核做FFT */
    fft(H);

    /* 准备输出 */
    const int sigLen = signal.size();
    QVector<double> output(sigLen, 0.0);
    int outPos = 0;

    /* 逐块处理 */
    int start = -(M - 1);  /* 从负偏移开始以包含前导零填充 */
    while (outPos < sigLen) {
        /* 构建输入块 */
        std::vector<std::complex<double>> X(N);

        for (int i = 0; i < N; ++i) {
            int sigIdx = start + i;
            if (sigIdx >= 0 && sigIdx < sigLen) {
                X[i] = signal[sigIdx];
            } else {
                X[i] = 0.0;  /* 零填充 */
            }
        }

        /* FFT → 乘以H → IFFT */
        fft(X);
        for (int i = 0; i < N; ++i) {
            X[i] *= H[i];
        }
        ifft(X);

        /* 取有效部分(丢弃前M-1个点) */
        for (int i = M - 1; i < N && outPos < sigLen; ++i) {
            output[outPos++] = X[i].real();
        }

        start += L;
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    const qint64 elapsed = timer.elapsed();
    const auto cnt = m_stats.totalProcessings;
    m_stats.avgProcessingTimeMs =
        m_stats.avgProcessingTimeMs * (cnt - 1) / cnt +
        static_cast<double>(elapsed) / cnt;

    emit processingCompleted(output.size());
    return output;
}

void OverlapSaveFFT::setBlockSize(int size)
{
    /* 至少64, 必须为2的幂 */
    m_blockSize = nextPowerOf2(std::max(size, 64));
}

int OverlapSaveFFT::blockSize() const
{
    return m_blockSize;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

OverlapSaveFFT::Stats OverlapSaveFFT::stats() const
{
    return m_stats;
}

void OverlapSaveFFT::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现: Cooley-Tukey FFT
// ═══════════════════════════════════════════════════════════

void OverlapSaveFFT::fft(std::vector<std::complex<double>>& data)
{
    const int n = static_cast<int>(data.size());
    if (n <= 1) return;

    /* 位逆序置换 */
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

    /* 蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        const double angle = -2.0 * M_PI / len;
        const std::complex<double> wn(std::cos(angle), std::sin(angle));

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
}

void OverlapSaveFFT::ifft(std::vector<std::complex<double>>& data)
{
    /* 共轭 → FFT → 共轭 → 除以N */
    const int n = static_cast<int>(data.size());
    for (int i = 0; i < n; ++i) {
        data[i] = std::conj(data[i]);
    }
    fft(data);
    for (int i = 0; i < n; ++i) {
        data[i] = std::conj(data[i]) / static_cast<double>(n);
    }
}

int OverlapSaveFFT::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) {
        p *= 2;
    }
    return p;
}
