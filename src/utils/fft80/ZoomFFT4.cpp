/**
 * @file ZoomFFT4.cpp
 * @brief 细化FFT(Zoom FFT)实现 — 频移+低通+抽取+FFT
 *
 * 细化FFT在指定频段内进行高分辨率频谱分析，通过频移(数字下变频)、
 * 低通滤波、抽取降采样和FFT四个步骤，等效于超长FFT在窄带内的效果。
 * 适用于机械故障诊断、谐波分析等需要精细频谱的嵌入式场景。
 *
 * 处理管线:
 * 1. 频率搬移: 混频将中心频率移至DC
 * 2. 低通滤波: FIR滤波器滤除带外分量
 * 3. 降采样(decimation): 降低采样率，提高等效分辨率
 * 4. FFT分析: 对降采样信号执行基2 FFT
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/fft80/ZoomFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <complex>

/**
 * @brief 构造函数，初始化默认细化参数
 * @param parent 父QObject指针
 */
ZoomFFT4::ZoomFFT4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置细化频段和输出点数
 * @param centerFreq 中心频率(Hz)
 * @param bandwidth 分析带宽(Hz)
 * @param outputPoints 输出FFT点数
 * @return 参数是否合法
 *
 * 最终频率分辨率 = bandwidth / outputPoints，
 * 等效FFT大小 = sampleRate / frequencyResolution。
 */
bool ZoomFFT4::setZoomRange(double centerFreq, double bandwidth, int outputPoints)
{
    if (bandwidth <= 0.0 || outputPoints <= 0) {
        return false;
    }
    m_centerFreq = qMax(0.0, centerFreq);
    m_bandwidth = bandwidth;
    m_outputPoints = outputPoints;
    return true;
}

/**
 * @brief 执行细化FFT分析
 * @param input 输入时域实数信号
 * @return 复数频谱结果(长度为outputPoints)
 *
 * 完整的Zoom FFT处理管线:
 * 1. 数字下变频(DDC): x[n]*e^{-j*2*pi*fc*n/fs} 将fc搬到DC
 * 2. FIR低通滤波: 截止频率 = bandwidth/2
 * 3. 抽取: 按decimationFactor降采样
 * 4. 基2 FFT: 对抽取后信号做频谱分析
 */
QVector<std::complex<double>> ZoomFFT4::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<std::complex<double>> result;

    if (input.size() < 4 || m_outputPoints <= 0 || m_bandwidth <= 0.0) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
            ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0, 0.0);
        return result;
    }

    const int N = input.size();
    const double sampleRate = static_cast<double>(m_equivalentSize > 0
        ? m_equivalentSize : N);

    /* 步骤1: 频率搬移(数字下变频DDC)
     * 将中心频率移到DC: y[n] = x[n] * e^{-j*2*pi*fc*n/fs}
     */
    QVector<std::complex<double>> shifted(N);
    for (int i = 0; i < N; ++i) {
        double phase = -2.0 * M_PI * m_centerFreq * i / sampleRate;
        shifted[i] = std::complex<double>(input[i] * qCos(phase),
                                           input[i] * qSin(phase));
    }

    /* 步骤2: FIR低通滤波
     * 截止频率 = bandwidth/2，使用sinc窗设计
     */
    int filterLen = qMin(63, qMax(15, N / 4));
    QVector<double> firCoeff = designLowpass(filterLen, m_bandwidth / sampleRate);

    QVector<std::complex<double>> filtered(N);
    for (int i = 0; i < N; ++i) {
        std::complex<double> sum(0.0, 0.0);
        for (int k = 0; k < filterLen; ++k) {
            int idx = i - k;
            if (idx >= 0 && idx < N) {
                sum += shifted[idx] * firCoeff[k];
            }
        }
        filtered[i] = sum;
    }

    /* 步骤3: 抽取(decimation)
     * 抽取因子 = fs / bandwidth (等效采样率降为bandwidth)
     */
    int decimFactor = qMax(1, static_cast<int>(sampleRate / m_bandwidth));
    if (decimFactor < 1) decimFactor = 1;

    QVector<std::complex<double>> decimated;
    decimated.reserve(N / decimFactor + 1);
    for (int i = 0; i < N; i += decimFactor) {
        decimated.append(filtered[i]);
    }

    /* 步骤4: 基2 FFT
     * 补零到outputPoints(2的幂)
     */
    int fftSize = nextPow2(qMax(m_outputPoints, decimated.size()));
    QVector<std::complex<double>> fftData(fftSize, std::complex<double>(0.0, 0.0));
    for (int i = 0; i < qMin(decimated.size(), fftSize); ++i) {
        fftData[i] = decimated[i];
    }

    fftInplace(fftData, false);

    /* 提取前outputPoints个结果 */
    result.resize(m_outputPoints);
    for (int i = 0; i < m_outputPoints && i < fftSize; ++i) {
        result[i] = fftData[i];
    }

    /* 更新统计信息 */
    double resolutionHz = m_bandwidth / m_outputPoints;
    m_stats.totalTransforms++;
    m_stats.totalZoomPoints += m_outputPoints;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_outputPoints, resolutionHz);
    return result;
}

/**
 * @brief 获取幅度谱
 * @param input 输入时域信号
 * @return 幅度谱向量(长度为outputPoints)
 *
 * 调用transform()后取各频率分量的模值。
 */
QVector<double> ZoomFFT4::magnitudeSpectrum(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<std::complex<double>> spectrum = transform(input);
    QVector<double> magnitude(spectrum.size());

    for (int i = 0; i < spectrum.size(); ++i) {
        magnitude[i] = std::abs(spectrum[i]);
    }

    /* 归一化: 除以分析点数 */
    if (!magnitude.isEmpty()) {
        double scale = 1.0 / qMax(1, input.size() / 2);
        for (int i = 0; i < magnitude.size(); ++i) {
            magnitude[i] *= scale;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalTransforms);

    return magnitude;
}

/**
 * @brief 设置等效FFT大小(影响频率分辨率计算)
 * @param size 等效FFT大小(用于确定虚拟采样率)
 */
void ZoomFFT4::setEquivalentSize(int size)
{
    m_equivalentSize = qMax(0, size);
}

/**
 * @brief 重置所有累计统计信息
 */
void ZoomFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设计FIR低通滤波器系数
 * @param length 滤波器长度
 * @param normalizedCutoff 归一化截止频率(0~0.5)
 * @return FIR滤波器系数
 *
 * 使用加窗sinc方法: h[n] = sinc(2*fc*(n-M)) * w[n]
 * 其中 w[n] 为汉宁窗，M = (length-1)/2
 */
QVector<double> ZoomFFT4::designLowpass(int length, double normalizedCutoff) const
{
    QVector<double> coeffs(length, 0.0);
    double cutoff = qBound(0.001, normalizedCutoff, 0.499);
    double mid = (length - 1) / 2.0;

    double sumSq = 0.0;
    for (int i = 0; i < length; ++i) {
        double t = static_cast<double>(i) - mid;
        double sincVal;
        if (qAbs(t) < 1e-10) {
            sincVal = 1.0;
        } else {
            sincVal = qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        }
        /* 汉宁窗 */
        double win = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (length - 1)));
        coeffs[i] = sincVal * win;
        sumSq += coeffs[i] * coeffs[i];
    }

    /* 归一化使增益为1 */
    if (sumSq > 1e-15) {
        double norm = 1.0 / qSqrt(sumSq);
        for (int i = 0; i < length; ++i) {
            coeffs[i] *= norm * 2.0;
        }
    }

    return coeffs;
}

/**
 * @brief 原地基2 FFT实现
 * @param data 复数数组(长度必须为2的幂)
 * @param inverse true执行IFFT
 */
void ZoomFFT4::fftInplace(QVector<std::complex<double>>& data, bool inverse) const
{
    const int n = data.size();
    if (n <= 1) return;

    /* 位反转排列 */
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
    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = sign * 2.0 * M_PI / len;
        std::complex<double> wLen(std::cos(angle), std::sin(angle));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wLen;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            data[i] /= n;
        }
    }
}

/**
 * @brief 计算大于等于n的最小2的幂
 * @param n 输入值
 * @return >= n 的最小2的幂
 */
int ZoomFFT4::nextPow2(int n) const
{
    int p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}
