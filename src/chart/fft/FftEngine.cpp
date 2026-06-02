/**
 * @file FftEngine.cpp
 * @brief FFT频谱计算引擎实现 -- Cooley-Tukey radix-2 DIT算法
 *
 * 实现 Cooley-Tukey 时间抽取（DIT）基-2 FFT算法，
 * 包含位反转置换和蝶形运算两个核心步骤。
 * 附带四种常用窗函数的实现。
 */

#include "chart/fft/FftEngine.h"

#include <QtMath>
#include <algorithm>

// ============================================================
// 构造 / 工具
// ============================================================

FftEngine::FftEngine(QObject* parent)
    : QObject(parent)
{
}

int FftEngine::nextPowerOf2(int n)
{
    if (n <= 0) {
        return 1;
    }
    // 如果已经是2的幂则直接返回
    if ((n & (n - 1)) == 0) {
        return n;
    }
    // 否则取最高位并左移一位
    int highest = 0;
    while (n > 0) {
        n >>= 1;
        ++highest;
    }
    return (1 << highest);
}

int FftEngine::log2Int(int n)
{
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        ++bits;
    }
    return bits;
}

int FftEngine::bitReverse(int index, int bits)
{
    int reversed = 0;
    for (int i = 0; i < bits; ++i) {
        reversed = (reversed << 1) | (index & 1);
        index >>= 1;
    }
    return reversed;
}

QString FftEngine::windowTypeName(WindowType window)
{
    switch (window) {
    case WindowType::Rectangular: return QStringLiteral("Rectangular");
    case WindowType::Hanning:     return QStringLiteral("Hanning");
    case WindowType::Hamming:     return QStringLiteral("Hamming");
    case WindowType::Blackman:    return QStringLiteral("Blackman");
    }
    return QStringLiteral("Unknown");
}

// ============================================================
// 主计算接口
// ============================================================

QVector<QPointF> FftEngine::compute(const QVector<QPointF>& timeData,
                                    double sampleRate,
                                    WindowType window,
                                    int fftSize)
{
    // 无数据时返回空频谱
    if (timeData.isEmpty() || sampleRate <= 0.0) {
        return {};
    }

    // 确定FFT长度: 用户指定 or 自动取nextPowerOf2
    const int N = (fftSize > 0) ? nextPowerOf2(fftSize)
                                : nextPowerOf2(timeData.size());

    // 构造复数序列，从时域数据的Y值提取
    QVector<std::complex<double>> data;
    data.reserve(N);
    for (int i = 0; i < N; ++i) {
        if (i < timeData.size()) {
            data.append(std::complex<double>(timeData[i].y(), 0.0));
        } else {
            // 零填充
            data.append(std::complex<double>(0.0, 0.0));
        }
    }

    // 1. 应用窗函数
    applyWindow(data, window);

    // 2. 执行FFT（原地）
    fftRadix2(data);

    // 3. 计算单边幅度谱
    QVector<QPointF> spectrum = magnitudeSpectrum(data, sampleRate);

    // 4. 查找基频（幅度最大处对应的频率）
    double fundamentalFreq = 0.0;
    double maxMag = 0.0;
    for (const auto& pt : spectrum) {
        if (pt.y() > maxMag) {
            maxMag = pt.y();
            fundamentalFreq = pt.x();
        }
    }

    emit spectrumComputed(spectrum, fundamentalFreq);
    return spectrum;
}

// ============================================================
// 窗函数
// ============================================================

void FftEngine::applyWindow(QVector<std::complex<double>>& data, WindowType window)
{
    const int N = data.size();
    if (N <= 1) {
        /* N=1时 (N-1)=0 导致除零; N=0无需处理 */
        return;
    }

    switch (window) {
    case WindowType::Rectangular:
        // 矩形窗: 不做任何处理（等效于全1）
        break;

    case WindowType::Hanning:
        // 汉宁窗: w(n) = 0.5 * (1 - cos(2πn/(N-1)))
        for (int n = 0; n < N; ++n) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
            data[n] *= w;
        }
        break;

    case WindowType::Hamming:
        // 海明窗: w(n) = 0.54 - 0.46 * cos(2πn/(N-1))
        for (int n = 0; n < N; ++n) {
            double w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
            data[n] *= w;
        }
        break;

    case WindowType::Blackman:
        // 布莱克曼窗: w(n) = 0.42 - 0.5*cos(2πn/(N-1)) + 0.08*cos(4πn/(N-1))
        for (int n = 0; n < N; ++n) {
            double w = 0.42
                     - 0.50 * qCos(2.0 * M_PI * n / (N - 1))
                     + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
            data[n] *= w;
        }
        break;
    }
}

// ============================================================
// Cooley-Tukey radix-2 DIT FFT
// ============================================================

void FftEngine::fftRadix2(QVector<std::complex<double>>& data)
{
    const int N = data.size();
    if (N <= 1) {
        return;
    }

    const int stages = log2Int(N);

    // 步骤1: 位反转置换
    // 将输入数据按照位反转顺序重排，这是DIT算法的前置步骤
    for (int i = 0; i < N; ++i) {
        int j = bitReverse(i, stages);
        if (j > i) {
            std::swap(data[i], data[j]);
        }
    }

    // 步骤2: 蝶形运算
    // 逐级（stage）合并子序列，每级的蝶形跨度（span）翻倍
    for (int stage = 1; stage <= stages; ++stage) {
        int span = (1 << stage);        // 当前级的蝶形跨度: 2^stage
        int halfSpan = span >> 1;       // 跨度的一半

        // 旋转因子步进角: e^(-j*2π/span)
        // 利用欧拉公式展开: cos(θ) - j*sin(θ)
        double angleStep = -2.0 * M_PI / span;
        double wReal = qCos(angleStep);
        double wImag = qSin(angleStep);

        // 遍历所有蝶形组
        for (int base = 0; base < N; base += span) {
            double curReal = 1.0;   // 旋转因子实部，初始 W^0 = 1
            double curImag = 0.0;   // 旋转因子虚部，初始 W^0 = 0

            for (int k = 0; k < halfSpan; ++k) {
                int top = base + k;
                int bot = top + halfSpan;

                // 蝶形运算:
                // X[bot] = data[top] - W * data[bot]
                // data[top] = data[top] + W * data[bot]
                double tReal = curReal * data[bot].real() - curImag * data[bot].imag();
                double tImag = curReal * data[bot].imag() + curImag * data[bot].real();

                data[bot] = std::complex<double>(
                    data[top].real() - tReal,
                    data[top].imag() - tImag);
                data[top] = std::complex<double>(
                    data[top].real() + tReal,
                    data[top].imag() + tImag);

                // 更新旋转因子: W *= w（复数乘法）
                double newReal = curReal * wReal - curImag * wImag;
                double newImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
                curImag = newImag;
            }
        }
    }
}

// ============================================================
// 单边幅度谱
// ============================================================

QVector<QPointF> FftEngine::magnitudeSpectrum(
    const QVector<std::complex<double>>& fftResult,
    double sampleRate)
{
    const int N = fftResult.size();
    if (N <= 1) {
        return {};
    }

    const int halfN = N / 2;
    const double freqResolution = sampleRate / N;

    QVector<QPointF> spectrum;
    spectrum.reserve(halfN);

    for (int k = 0; k < halfN; ++k) {
        double magnitude = std::abs(fftResult[k]);

        // 归一化: 直流分量(k=0)和奈奎斯特分量(k=N/2)不乘2，其余乘2
        if (k > 0 && k < halfN) {
            magnitude *= 2.0;
        }
        magnitude /= N;

        double freq = k * freqResolution;
        spectrum.append(QPointF(freq, magnitude));
    }

    return spectrum;
}
