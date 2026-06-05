/**
 * @file HilbertTransform.cpp
 * @brief 离散Hilbert变换实现 — FFT构造解析信号 + 包络/瞬时频率
 */

#include "utils/fft14/HilbertTransform.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/* ── 构造 ── */

/** @brief 构造函数 @param parent 父对象 */
HilbertTransform::HilbertTransform(QObject* parent)
    : QObject(parent)
    , m_boundaryMode(BoundaryMode::None)
{
}

/** @brief 设置边界处理模式 @param mode 模式 */
void HilbertTransform::setBoundaryMode(BoundaryMode mode)
{
    m_boundaryMode = mode;
}

/* ── 核心变换 ── */

/** @brief 计算解析信号(实部+虚部) @param data 输入信号 @return 解析信号结果 */
HilbertTransform::AnalyticResult HilbertTransform::compute(
    const QVector<double>& data)
{
    AnalyticResult result;
    if (data.size() < 4) return result;

    QElapsedTimer timer;
    timer.start();

    int n = data.size();

    /* 边界校正 */
    QVector<double> padded = applyBoundary(data);
    int N = padded.size();

    /* 零填充到2的幂 */
    int fftSize = nextPowerOf2(N);
    QVector<double> real(fftSize, 0.0);
    QVector<double> imag(fftSize, 0.0);
    for (int i = 0; i < N; ++i) real[i] = padded[i];

    /* 前向FFT */
    fftInPlace(real, imag, false);

    /* 构造Hilbert滤波器: 正频率分量乘2，直流和Nyquist不变 */
    real[0] *= 1.0;
    imag[0] *= 1.0;
    for (int i = 1; i < fftSize / 2; ++i) {
        real[i] *= 2.0;
        imag[i] *= 2.0;
    }
    /* Nyquist分量保持不变 */
    real[fftSize / 2] *= 1.0;
    imag[fftSize / 2] *= 1.0;
    /* 负频率分量置零 */
    for (int i = fftSize / 2 + 1; i < fftSize; ++i) {
        real[i] = 0.0;
        imag[i] = 0.0;
    }

    /* 逆FFT回到时域 */
    fftInPlace(real, imag, true);

    /* 提取原始长度 */
    result.analyticReal.resize(n);
    result.analyticImag.resize(n);
    result.envelope.resize(n);
    result.instantaneousPhase.resize(n);
    result.instantaneousFreq.resize(n);

    for (int i = 0; i < n; ++i) {
        result.analyticReal[i] = data[i];      /* 原始信号作为实部 */
        result.analyticImag[i] = imag[i];       /* Hilbert变换作为虚部 */
        result.envelope[i] = std::sqrt(data[i] * data[i] + imag[i] * imag[i]);
        result.instantaneousPhase[i] = std::atan2(imag[i], data[i]);
    }

    /* 计算瞬时频率(相位的导数) */
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            result.instantaneousFreq[i] = result.instantaneousPhase[0];
        } else {
            double dp = result.instantaneousPhase[i]
                      - result.instantaneousPhase[i - 1];
            /* 相位展开: 确保差值在[-pi,pi] */
            while (dp > M_PI) dp -= 2.0 * M_PI;
            while (dp < -M_PI) dp += 2.0 * M_PI;
            result.instantaneousFreq[i] = dp;
        }
    }

    /* 更新统计 */
    ++m_stats.totalTransforms;
    m_stats.totalSamplesProcessed += n;
    m_stats.totalFFTSizeUsed += fftSize;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformComplete(n);
    return result;
}

/** @brief 仅计算包络 @param data 输入信号 @return 包络 */
QVector<double> HilbertTransform::envelope(const QVector<double>& data)
{
    if (data.size() < 4) return {};
    return compute(data).envelope;
}

/** @brief 仅计算瞬时相位 @param data 输入信号 @return 瞬时相位(rad) */
QVector<double> HilbertTransform::instantaneousPhase(
    const QVector<double>& data)
{
    if (data.size() < 4) return {};
    return compute(data).instantaneousPhase;
}

/** @brief 计算瞬时频率(Hz) @param data 输入信号 @param sampleRate 采样率 @return 瞬时频率 */
QVector<double> HilbertTransform::instantaneousFrequency(
    const QVector<double>& data, double sampleRate)
{
    if (data.size() < 4 || sampleRate <= 0.0) return {};

    auto result = compute(data);
    QVector<double> freq(result.instantaneousFreq.size());
    double scale = sampleRate / (2.0 * M_PI);
    for (int i = 0; i < freq.size(); ++i) {
        freq[i] = result.instantaneousFreq[i] * scale;
    }
    return freq;
}

/* ── 私有工具 ── */

/** @brief 下一个2的幂 */
int HilbertTransform::nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/** @brief 基2就地FFT(Cooley-Tukey) @param real 实部数组 @param imag 虚部数组 @param inverse 是否逆变换 */
void HilbertTransform::fftInPlace(QVector<double>& real,
                                   QVector<double>& imag, bool inverse)
{
    int n = real.size();
    if (n <= 1) return;

    /* 比特反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * M_PI / static_cast<double>(len)
                   * (inverse ? -1.0 : 1.0);
        double wReal = std::cos(ang);
        double wImag = std::sin(ang);

        for (int i = 0; i < n; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tReal = curReal * real[v] - curImag * imag[v];
                double tImag = curReal * imag[v] + curImag * real[v];
                real[v] = real[u] - tReal;
                imag[v] = imag[u] - tImag;
                real[u] += tReal;
                imag[u] += tImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }

    /* 逆变换归一化 */
    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= static_cast<double>(n);
            imag[i] /= static_cast<double>(n);
        }
    }
}

/** @brief 边界校正 @param data 输入数据 @return 校正后数据 */
QVector<double> HilbertTransform::applyBoundary(const QVector<double>& data)
{
    if (m_boundaryMode == BoundaryMode::None) return data;

    int n = data.size();
    int pad = qMax(4, n / 4);
    QVector<double> padded;
    padded.reserve(n + 2 * pad);

    if (m_boundaryMode == BoundaryMode::Reflect) {
        /* 镜像反射: [data[n-1]...data[0], data, data[n-1]...data[0]] */
        for (int i = pad; i > 0; --i) {
            padded.append(data[qMin(i, n - 1)]);
        }
        for (int i = 0; i < n; ++i) padded.append(data[i]);
        for (int i = 1; i <= pad; ++i) {
            padded.append(data[qMax(n - 1 - i, 0)]);
        }
    } else {
        /* 余弦锥化: 两端加余弦窗渐变 */
        for (int i = 0; i < pad; ++i) {
            double w = 0.5 * (1.0 - std::cos(M_PI * i / static_cast<double>(pad)));
            padded.append(data[0] * w);
        }
        for (int i = 0; i < n; ++i) padded.append(data[i]);
        for (int i = 0; i < pad; ++i) {
            double w = 0.5 * (1.0 + std::cos(M_PI * i / static_cast<double>(pad)));
            padded.append(data[n - 1] * w);
        }
    }
    return padded;
}

/* ── 统计 ── */

/** @brief 重置统计 */
void HilbertTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
