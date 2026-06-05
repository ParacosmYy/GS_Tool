/**
 * @file EnvelopeDetect.cpp
 * @brief 包络检测引擎实现 — Hilbert包络/峰值插值/Attack-Release/RMS包络
 */

#include "utils/signal22/EnvelopeDetect.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
EnvelopeDetect::EnvelopeDetect(QObject* parent)
    : QObject(parent)
    , m_type(EnvelopeType::Hilbert)
    , m_attackTime(10.0)
    , m_releaseTime(50.0)
    , m_rmsWindowSize(256)
    , m_sampleRate(44100.0)
    , m_envelopeState(0.0)
    , m_attackCoeff(0.0)
    , m_releaseCoeff(0.0)
    , m_timeSum(0.0)
{
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_attackTime * 0.001));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_releaseTime * 0.001));
}

/** @brief 设置包络类型 @param type 包络检测类型 */
void EnvelopeDetect::setEnvelopeType(EnvelopeType type)
{
    m_type = type;
}

/** @brief 设置Attack时间 @param ms Attack时间(ms) */
void EnvelopeDetect::setAttackTime(double ms)
{
    m_attackTime = qMax(0.01, ms);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_attackTime * 0.001));
}

/** @brief 设置Release时间 @param ms Release时间(ms) */
void EnvelopeDetect::setReleaseTime(double ms)
{
    m_releaseTime = qMax(0.01, ms);
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_releaseTime * 0.001));
}

/** @brief 设置RMS窗口大小 @param size 窗口采样数 */
void EnvelopeDetect::setRmsWindowSize(int size)
{
    m_rmsWindowSize = qMax(4, size);
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void EnvelopeDetect::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_attackTime * 0.001));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_releaseTime * 0.001));
}

/** @brief 检测信号包络 @param data 输入信号 @return 包络结果 */
EnvelopeDetect::EnvelopeResult EnvelopeDetect::detect(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    EnvelopeResult result;
    if (data.isEmpty()) return result;

    switch (m_type) {
    case EnvelopeType::Hilbert:
        result.envelope = hilbertEnvelope(data);
        break;
    case EnvelopeType::Peak:
        result.envelope.resize(data.size(), 0.0);
        {
            /* 峰值包络: 取绝对值后的峰值保持 */
            double peak = 0.0;
            for (int i = 0; i < data.size(); ++i) {
                double absVal = qAbs(data[i]);
                if (absVal > peak) {
                    peak = absVal;
                } else {
                    /* 峰值衰减 */
                    peak *= 0.999;
                }
                result.envelope[i] = peak;
            }
        }
        break;
    case EnvelopeType::RMS:
        result.envelope.resize(data.size(), 0.0);
        {
            int halfWin = m_rmsWindowSize / 2;
            for (int i = 0; i < data.size(); ++i) {
                int start = qMax(0, i - halfWin);
                int end = qMin(data.size() - 1, i + halfWin);
                result.envelope[i] = computeRms(data, start, end - start + 1);
            }
        }
        break;
    case EnvelopeType::Smoothed:
        result.envelope = attackReleaseSmooth(
            hilbertEnvelope(data));
        break;
    }

    /* 检测峰值 */
    result.peaks = detectPeaks(result.envelope);

    /* 计算峰值幅度 */
    result.peakAmplitude = 0.0;
    for (double v : data) {
        if (qAbs(v) > result.peakAmplitude) {
            result.peakAmplitude = qAbs(v);
        }
    }

    /* 计算RMS电平 */
    result.rmsLevel = computeRms(data, 0, data.size());

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.totalSamplesProcessed += static_cast<quint64>(data.size());
    m_stats.totalPeaksDetected += static_cast<quint64>(result.peaks.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    /* 平均峰值幅度 */
    if (!result.peaks.isEmpty()) {
        double peakSum = 0.0;
        for (const auto& p : result.peaks) peakSum += p.value;
        double avgPeak = peakSum / result.peaks.size();
        m_stats.avgPeakAmplitude += (avgPeak - m_stats.avgPeakAmplitude)
            / static_cast<double>(m_stats.totalDetections);
    }

    emit detectionComplete(data.size(),
                           result.peaks.size(), result.peakAmplitude);
    return result;
}

/** @brief Hilbert变换计算瞬时包络 @param data 输入信号 @return 解析信号幅度 */
QVector<double> EnvelopeDetect::hilbertEnvelope(const QVector<double>& data)
{
    int n = data.size();
    if (n < 4) return data;

    /* 补零到2的幂 */
    int fftSize = 1;
    while (fftSize < n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);
    for (int i = 0; i < n; ++i) real[i] = data[i];

    /* 正向FFT */
    fft(real, imag);

    /* 构造Hilbert变换滤波器:
     * 正频率 * 2, 负频率 * 0, DC和Nyquist * 1 */
    int halfN = fftSize / 2;
    for (int i = 1; i < halfN; ++i) {
        real[i] *= 2.0;
        imag[i] *= 2.0;
    }
    for (int i = halfN + 1; i < fftSize; ++i) {
        real[i] = 0.0;
        imag[i] = 0.0;
    }

    /* 逆FFT: 先取共轭再做FFT, 再取共轭除以N */
    for (int i = 0; i < fftSize; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < fftSize; ++i) {
        real[i] /= fftSize;
        imag[i] = -imag[i] / fftSize;
    }

    /* 计算瞬时包络 = sqrt(real^2 + imag^2) */
    QVector<double> envelope(n);
    for (int i = 0; i < n; ++i) {
        envelope[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
    }

    return envelope;
}

/** @brief 带插值的峰值检测 @param data 输入信号 @return 峰值列表 */
QVector<EnvelopeDetect::PeakInfo> EnvelopeDetect::detectPeaks(
    const QVector<double>& data) const
{
    QVector<PeakInfo> peaks;
    if (data.size() < 3) return peaks;

    /* 寻找局部极大值 */
    double threshold = 0.0;
    double maxVal = *std::max_element(data.begin(), data.end());
    threshold = maxVal * 0.05; /* 5%阈值过滤噪声峰 */

    for (int i = 1; i < data.size() - 1; ++i) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]
            && data[i] > threshold) {
            peaks.append(interpolatePeak(data, i));
        }
    }

    return peaks;
}

/** @brief Attack/Release平滑 @param data 输入信号 @return 平滑后的信号 */
QVector<double> EnvelopeDetect::attackReleaseSmooth(
    const QVector<double>& data)
{
    QVector<double> result(data.size());
    double state = m_envelopeState;

    for (int i = 0; i < data.size(); ++i) {
        double input = qAbs(data[i]);
        if (input > state) {
            /* Attack */
            state += m_attackCoeff * (input - state);
        } else {
            /* Release */
            state += m_releaseCoeff * (input - state);
        }
        result[i] = state;
    }

    m_envelopeState = state;
    return result;
}

/** @brief 重置统计 */
void EnvelopeDetect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelopeState = 0.0;
}

/** @brief 基2 FFT @param real 实部 @param imag 虚部 */
void EnvelopeDetect::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* FFT蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j, odd = i + j + len / 2;
                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];
                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/** @brief 抛物线插值峰值 @param data 数据 @param idx 峰值索引 @return 插值后的峰值信息 */
EnvelopeDetect::PeakInfo EnvelopeDetect::interpolatePeak(
    const QVector<double>& data, int idx) const
{
    PeakInfo info;
    info.index = idx;
    info.value = data[idx];

    if (idx <= 0 || idx >= data.size() - 1) {
        info.interpolatedIndex = static_cast<double>(idx);
        info.interpolatedValue = data[idx];
        return info;
    }

    /* 三点抛物线插值 */
    double y0 = data[idx - 1];
    double y1 = data[idx];
    double y2 = data[idx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) > 1e-14) {
        double delta = (y0 - y2) / denom;
        info.interpolatedIndex = static_cast<double>(idx) + delta;
        info.interpolatedValue = y1 - (y0 - y2) * delta / 4.0;
    } else {
        info.interpolatedIndex = static_cast<double>(idx);
        info.interpolatedValue = y1;
    }

    return info;
}

/** @brief 计算RMS @param data 数据 @param start 起始位置 @param len 长度 @return RMS值 */
double EnvelopeDetect::computeRms(const QVector<double>& data,
                                   int start, int len) const
{
    if (len <= 0 || start < 0 || start + len > data.size()) return 0.0;
    double sum = 0.0;
    for (int i = start; i < start + len; ++i) {
        sum += data[i] * data[i];
    }
    return qSqrt(sum / len);
}
