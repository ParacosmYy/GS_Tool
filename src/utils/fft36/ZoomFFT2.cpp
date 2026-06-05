/**
 * @file ZoomFFT2.cpp
 * @brief Zoom-FFT 实现 — 频率搬移 + 低通滤波 + 抽取 + FFT
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Zoom FFT（细化 FFT）算法，用于对指定频带进行高分辨率频谱分析：
 * 1. 频率搬移：将中心频率搬移到零频
 * 2. 低通滤波：滤除带宽外的频率分量
 * 3. 抽取（降采样）：降低采样率
 * 4. FFT：对抽取后的信号进行频谱分析
 */

#include "utils/fft36/ZoomFFT2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
ZoomFFT2::ZoomFFT2(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ZoomFFT2"));
}

/**
 * @brief 重置统计信息
 */
void ZoomFFT2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设置采样率
 * @param rate 采样率 (Hz)
 */
void ZoomFFT2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置中心频率
 * @param freq 中心频率 (Hz)，即需要细化的频带中心
 */
void ZoomFFT2::setCenterFreq(double freq)
{
    m_centerFreq = qBound(0.0, freq, m_sampleRate / 2.0);
}

/**
 * @brief 设置分析带宽
 * @param bw 带宽 (Hz)，细化频带的宽度
 */
void ZoomFFT2::setBandwidth(double bw)
{
    m_bandwidth = qBound(1.0, bw, m_sampleRate / 2.0);
}

/**
 * @brief 设置输出频率分辨率对应的 FFT 点数
 * @param bins FFT 点数
 */
void ZoomFFT2::setOutputBins(int bins)
{
    m_outputBins = qMax(16, bins);
}

/**
 * @brief 计算频率分辨率
 * @return 每个频率 bin 的宽度 (Hz)
 */
double ZoomFFT2::frequencyResolution() const
{
    return m_bandwidth / m_outputBins;
}

/**
 * @brief 基-2 原地 FFT
 *
 * Cooley-Tukey 算法，输入为交替实虚部的交错序列。
 *
 * @param re 实部数组
 * @param im 虚部数组
 * @param n FFT 点数（必须为2的幂）
 * @param inverse true 执行逆 FFT
 */
static void fftImpl(QVector<double> &re, QVector<double> &im, int n, bool inverse)
{
    // 位反转置换
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
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

    // 蝶形运算
    for (int len = 2; len <= n; len <<= 1) {
        const double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        const double wRe = qCos(angle);
        const double wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                const double tRe = curRe * re[i + j + len / 2] - curIm * im[i + j + len / 2];
                const double tIm = curRe * im[i + j + len / 2] + curIm * re[i + j + len / 2];

                re[i + j + len / 2] = re[i + j] - tRe;
                im[i + j + len / 2] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;

                const double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            re[i] /= n;
            im[i] /= n;
        }
    }
}

/**
 * @brief 设计并应用 FIR 低通滤波器
 *
 * 使用窗函数法设计截止频率为 bandwidth/2 的低通滤波器。
 * 滤波器长度由过渡带宽决定，使用 Hamming 窗。
 *
 * @param signal 输入信号（实虚交错）
 * @param cutoffNorm 归一化截止频率 (0, 0.5]
 * @param filterLen 滤波器长度
 * @return 滤波后的信号
 */
static QVector<double> applyLowpassFilter(const QVector<double> &signal,
                                           double cutoffNorm, int filterLen)
{
    // 设计 sinc 低通滤波器系数
    QVector<double> h(filterLen, 0.0);
    const int mid = filterLen / 2;
    double sumH = 0.0;

    for (int i = 0; i < filterLen; ++i) {
        const int n = i - mid;
        if (n == 0) {
            h[i] = 2.0 * cutoffNorm;
        } else {
            h[i] = qSin(2.0 * M_PI * cutoffNorm * n) / (M_PI * n);
        }
        // Hamming 窗
        const double window = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (filterLen - 1));
        h[i] *= window;
        sumH += qFabs(h[i]);
    }

    // 归一化滤波器
    if (sumH > 1e-12) {
        for (int i = 0; i < filterLen; ++i) {
            h[i] /= sumH;
        }
    }

    // 卷积（仅实部）
    const int sigLen = signal.size();
    QVector<double> filtered(sigLen, 0.0);
    for (int i = 0; i < sigLen; ++i) {
        double acc = 0.0;
        for (int j = 0; j < filterLen; ++j) {
            const int idx = i - j;
            if (idx >= 0 && idx < sigLen) {
                acc += signal[idx] * h[j];
            }
        }
        filtered[i] = acc;
    }

    return filtered;
}

/**
 * @brief 执行 Zoom FFT 变换
 *
 * 处理流程：
 * 1. 频率搬移：乘以 exp(-j*2*pi*fc*n/fs) 将中心频率搬移到零频
 * 2. 低通滤波：用 FIR 滤波器滤除带宽外分量
 * 3. 抽取：按降采样因子 D 抽取
 * 4. FFT：对抽取后的信号做 N 点 FFT
 * 5. 计算幅度谱
 *
 * @param input 输入时域信号
 * @return 细化频段的幅度谱
 */
QVector<double> ZoomFFT2::transform(const QVector<double> &input)
{
    QElapsedTimer timer;
    timer.start();

    const int inputLen = input.size();
    if (inputLen < 4) {
        return QVector<double>(m_outputBins, 0.0);
    }

    // 降采样因子
    const double decimationFactor = m_sampleRate / m_bandwidth;
    const int D = qMax(1, static_cast<int>(qRound(decimationFactor)));

    // 步骤1：频率搬移 — 乘以复指数
    const double phaseStep = -2.0 * M_PI * m_centerFreq / m_sampleRate;
    QVector<double> shiftedRe(inputLen);
    QVector<double> shiftedIm(inputLen);

    for (int i = 0; i < inputLen; ++i) {
        const double phase = phaseStep * i;
        shiftedRe[i] = input[i] * qCos(phase);
        shiftedIm[i] = input[i] * qSin(phase);
    }

    // 步骤2：低通滤波
    const double cutoffNorm = (m_bandwidth / 2.0) / m_sampleRate;
    const int filterLen = qMin(127, qMax(15, inputLen / 8));
    shiftedRe = applyLowpassFilter(shiftedRe, cutoffNorm, filterLen);
    shiftedIm = applyLowpassFilter(shiftedIm, cutoffNorm, filterLen);

    // 步骤3：抽取
    const int decimatedLen = inputLen / D;
    if (decimatedLen < 2) {
        return QVector<double>(m_outputBins, 0.0);
    }

    QVector<double> decRe(m_outputBins, 0.0);
    QVector<double> decIm(m_outputBins, 0.0);

    for (int i = 0; i < m_outputBins && i * D < decimatedLen; ++i) {
        decRe[i] = shiftedRe[i * D];
        decIm[i] = shiftedIm[i * D];
    }

    // 步骤4：FFT
    fftImpl(decRe, decIm, m_outputBins, false);

    // 步骤5：计算幅度谱
    QVector<double> magnitude(m_outputBins / 2);
    for (int i = 0; i < m_outputBins / 2; ++i) {
        magnitude[i] = qSqrt(decRe[i] * decRe[i] + decIm[i] * decIm[i]);
    }

    // 更新统计信息
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += inputLen;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformComplete(magnitude.size());
    return magnitude;
}
