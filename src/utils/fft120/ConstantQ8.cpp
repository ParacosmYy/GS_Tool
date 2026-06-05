#include "ConstantQ8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化CQT v8引擎
 * @param parent 父对象指针
 */
ConstantQ8::ConstantQ8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ConstantQ8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置CQT参数
 * @param minFreqHz 最低频率(Hz)
 * @param maxFreqHz 最高频率(Hz)
 * @param binsPerOctave 每倍频程频点数
 * @param sampleRate 采样率(Hz)
 */
void ConstantQ8::setParameters(double minFreqHz, double maxFreqHz,
                                int binsPerOctave, double sampleRate)
{
    m_minFreq = qMax(1.0, minFreqHz);
    m_maxFreq = qMax(m_minFreq + 1.0, maxFreqHz);
    m_binsPerOctave = qMax(1, binsPerOctave);
    m_sampleRate = qMax(1.0, sampleRate);
    m_kernelComputed = false;
    m_centerFreqs.clear();
}

/**
 * @brief 获取各频段中心频率
 * @return 中心频率列表(Hz)
 */
QVector<double> ConstantQ8::centerFrequencies() const
{
    if (!m_centerFreqs.isEmpty()) return m_centerFreqs;

    QVector<double> freqs;
    double ratio = qPow(2.0, 1.0 / m_binsPerOctave);
    double freq = m_minFreq;
    while (freq <= m_maxFreq) {
        freqs.append(freq);
        freq *= ratio;
    }
    return freqs;
}

/**
 * @brief 就地基2 FFT
 */
static void radix2FFT(QVector<double>& re, QVector<double>& im)
{
    const int n = re.size();
    if (n <= 1) return;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wnRe = qCos(ang), wnIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double wRe = 1.0, wIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = wRe * re[i+j+len/2] - wIm * im[i+j+len/2];
                double tIm = wRe * im[i+j+len/2] + wIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double nw = wRe * wnRe - wIm * wnIm;
                wIm = wRe * wnIm + wIm * wnRe;
                wRe = nw;
            }
        }
    }
}

/**
 * @brief 计算CQT核矩阵
 *
 * 为每个频段预计算稀疏核（窗函数×复指数），用于快速CQT变换。
 *
 * @return 是否计算成功
 */
bool ConstantQ8::precomputeKernel()
{
    double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);

    /* 重建中心频率 */
    m_centerFreqs.clear();
    double ratio = qPow(2.0, 1.0 / m_binsPerOctave);
    double freq = m_minFreq;
    while (freq <= m_maxFreq) {
        m_centerFreqs.append(freq);
        freq *= ratio;
    }

    int numBins = m_centerFreqs.size();
    if (numBins == 0) return false;

    /* 最大FFT长度（对应最低频率） */
    int maxFFTLen = qMin(static_cast<int>(qRound(Q * m_sampleRate / m_minFreq)), 65536);
    int fftLen = 1;
    while (fftLen < maxFFTLen) fftLen <<= 1;

    /* 计算稀疏核 */
    m_kernel.clear();
    m_kernel.resize(numBins);

    for (int k = 0; k < numBins; ++k) {
        int windowLen = qMin(static_cast<int>(qRound(Q * m_sampleRate / m_centerFreqs[k])), fftLen);
        if (windowLen <= 0) continue;

        QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
        for (int n = 0; n < windowLen; ++n) {
            double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * n / windowLen));
            double angle = -2.0 * M_PI * m_centerFreqs[k] * n / m_sampleRate;
            re[n] = hann * qCos(angle) / windowLen;
            im[n] = hann * qSin(angle) / windowLen;
        }

        radix2FFT(re, im);

        /* 稀疏化：只保留显著值 */
        m_kernel[k].clear();
        m_kernel[k].reserve(fftLen);
        for (int i = 0; i < fftLen; ++i) {
            double mag = qSqrt(re[i] * re[i] + im[i] * im[i]);
            if (mag > 1e-8)
                m_kernel[k].append({re[i], im[i]});
            else
                m_kernel[k].append({0.0, 0.0});
        }
    }

    m_kernelComputed = true;
    return true;
}

/**
 * @brief 执行常数Q变换
 *
 * 如果已预计算核矩阵则使用快速路径（FFT×核矩阵点乘），
 * 否则使用逐频段窗函数DFT的慢速路径。
 *
 * @param signal 输入时域信号
 * @return CQT系数矩阵 (频段 × 时间帧)
 */
QVector<QVector<double>> ConstantQ8::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> result;
    if (signal.isEmpty()) {
        emit transformCompleted(0);
        return result;
    }

    auto freqs = centerFrequencies();
    int numBins = freqs.size();
    if (numBins == 0) {
        emit transformCompleted(0);
        return result;
    }

    /* 单帧分析 */
    result.resize(numBins);

    if (m_kernelComputed && !m_kernel.isEmpty()) {
        /* 快速路径：FFT + 核矩阵点乘 */
        int fftLen = m_kernel[0].size();
        if (fftLen == 0) fftLen = 1;

        QVector<double> re(fftLen, 0.0), im(fftLen, 0.0);
        for (int i = 0; i < qMin(signal.size(), fftLen); ++i) {
            double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * i / fftLen));
            re[i] = signal[i] * hann;
        }
        radix2FFT(re, im);

        for (int k = 0; k < numBins && k < m_kernel.size(); ++k) {
            double power = 0.0;
            for (int i = 0; i < fftLen && i < m_kernel[k].size(); ++i) {
                double kr = m_kernel[k][i].first;
                double ki = m_kernel[k][i].second;
                power += (re[i] * kr - im[i] * ki) * (re[i] * kr - im[i] * ki)
                       + (re[i] * ki + im[i] * kr) * (re[i] * ki + im[i] * kr);
            }
            result[k].append(qSqrt(power));
        }
    } else {
        /* 慢速路径：逐频段窗函数DFT */
        double Q = 1.0 / (qPow(2.0, 1.0 / m_binsPerOctave) - 1.0);
        for (int k = 0; k < numBins; ++k) {
            int windowLen = qMin(static_cast<int>(qRound(Q * signal.size() * freqs[k] / freqs[numBins - 1])),
                                  signal.size());
            if (windowLen <= 0) windowLen = qMin(64, signal.size());

            double power = 0.0;
            for (int n = 0; n < windowLen && n < signal.size(); ++n) {
                double hann = 0.5 * (1.0 - qCos(2.0 * M_PI * n / windowLen));
                double angle = -2.0 * M_PI * freqs[k] * n / m_sampleRate;
                double cRe = hann * qCos(angle);
                double cIm = hann * qSin(angle);
                power += (signal[n] * cRe) * (signal[n] * cRe)
                       + (signal[n] * cIm) * (signal[n] * cIm);
            }
            result[k].append(qSqrt(power));
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(numBins);
    return result;
}
