/**
 * @file SpectrumMonitor.cpp
 * @brief 频谱监控引擎核心实现 — FFT/加窗/峰值保持/瀑布图累积
 *
 * 从 SpectrumMonitor.cpp 拆分，包含所有计算逻辑。
 * 统计 getter 见 SpectrumMonitorStats.cpp。
 */

#include "chart/spectrum/SpectrumMonitor.h"

#include <QDebug>
#include <QtMath>
#include <algorithm>

// ============================================================
// 构造
// ============================================================

/** @brief 构造频谱监控引擎 */
SpectrumMonitor::SpectrumMonitor(QObject* parent)
    : QObject(parent)
{
    m_elapsedTimer.start();
    generateWindow(m_params.fftSize, m_params.window);
}

// ============================================================
// 参数配置
// ============================================================

/** @brief 设置频谱分析参数 */
void SpectrumMonitor::setParams(const SpectrumParams& params)
{
    m_params = params;
    generateWindow(m_params.fftSize, m_params.window);
    m_sampleBuffer.clear();
    clearPeakHold();
    clearSpectrogram();
}

/** @brief 获取当前参数 */
SpectrumParams SpectrumMonitor::params() const
{
    return m_params;
}

// ============================================================
// 数据输入
// ============================================================

/** @brief 输入一批采样数据 */
void SpectrumMonitor::feedData(const QVector<double>& samples)
{
    if (samples.isEmpty()) return;
    m_sampleBuffer.append(samples);
    m_totalSamplesProcessed += static_cast<quint64>(samples.size());
    processBuffer();
}

// ============================================================
// 数据输出
// ============================================================

/** @brief 获取最新频谱切片 */
SpectrumSlice SpectrumMonitor::lastSlice() const
{
    return m_lastSlice;
}

/** @brief 获取峰值保持频谱(dBFS) */
QVector<double> SpectrumMonitor::peakHoldMagnitudes() const
{
    return m_peakHold;
}

/** @brief 获取瀑布图数据 */
QVector<QVector<double>> SpectrumMonitor::spectrogram() const
{
    return m_spectrogram;
}

/** @brief 获取瀑布图行数 */
int SpectrumMonitor::spectrogramRows() const
{
    return m_spectrogram.size();
}

/** @brief 获取频率轴 */
QVector<double> SpectrumMonitor::frequencyAxis() const
{
    return m_lastSlice.freqs;
}

// ============================================================
// 控制
// ============================================================

/** @brief 清除峰值保持数据 */
void SpectrumMonitor::clearPeakHold()
{
    m_peakHold.clear();
}

/** @brief 清除瀑布图数据 */
void SpectrumMonitor::clearSpectrogram()
{
    m_spectrogram.clear();
}

/** @brief 清除所有数据 */
void SpectrumMonitor::clearAll()
{
    m_sampleBuffer.clear();
    clearPeakHold();
    clearSpectrogram();
    m_lastSlice = SpectrumSlice();
}

// ============================================================
// 处理缓冲区 — 按overlap取帧执行FFT
// ============================================================

/** @brief 处理缓冲区数据 */
void SpectrumMonitor::processBuffer()
{
    const int fftSize = m_params.fftSize;
    if (fftSize <= 0 || m_sampleBuffer.size() < fftSize) return;

    const int hopSize = static_cast<int>(fftSize * (1.0 - m_params.overlap));
    if (hopSize <= 0) return;

    while (m_sampleBuffer.size() >= fftSize) {
        QVector<double> frame(fftSize);
        std::copy(m_sampleBuffer.constBegin(),
                  m_sampleBuffer.constBegin() + fftSize,
                  frame.begin());

        SpectrumSlice slice = computeSpectrum(frame);
        m_lastSlice = slice;
        m_totalFramesProcessed++;

        updatePeakHold(slice.magnitudes);
        appendSpectrogramRow(slice.magnitudes);

        emit spectrumUpdated(slice);
        emit spectrogramUpdated();

        m_sampleBuffer.erase(m_sampleBuffer.begin(),
                             m_sampleBuffer.begin() + hopSize);
    }
}

// ============================================================
// FFT频谱计算
// ============================================================

/** @brief 执行单次FFT并生成频谱切片 */
SpectrumSlice SpectrumMonitor::computeSpectrum(const QVector<double>& frame)
{
    const int N = frame.size();
    const int halfN = N / 2 + 1;
    const double sampleRate = m_params.sampleRate;

    // 加窗 + 转复数
    QVector<std::complex<double>> data(N);
    for (int i = 0; i < N; ++i) {
        double w = (i < m_windowCoeffs.size()) ? m_windowCoeffs[i] : 1.0;
        data[i] = std::complex<double>(frame[i] * w, 0.0);
    }

    // FFT
    fftRadix2(data);
    m_totalFftExecuted++;

    // 幅度谱
    QVector<double> mags = computeMagnitudes(data);

    // 频率轴
    SpectrumSlice slice;
    slice.freqs.resize(halfN);
    slice.magnitudes = mags;
    slice.timestamp = static_cast<double>(m_elapsedTimer.elapsed());

    const double binWidth = sampleRate / static_cast<double>(N);
    for (int i = 0; i < halfN; ++i) {
        slice.freqs[i] = i * binWidth;
    }

    // 峰值检测
    double maxVal = -120.0;
    double maxFreq = 0.0;
    for (int i = 0; i < mags.size(); ++i) {
        if (mags[i] > maxVal) {
            maxVal = mags[i];
            maxFreq = slice.freqs[i];
        }
    }
    m_peakFrequency = maxFreq;
    m_peakMagnitude = maxVal;

    return slice;
}

// ============================================================
// 窗函数生成
// ============================================================

/** @brief 生成窗函数系数 */
void SpectrumMonitor::generateWindow(int size, WindowFunction wf)
{
    m_windowCoeffs.resize(size);
    const double N = static_cast<double>(size - 1);

    for (int i = 0; i < size; ++i) {
        double n = static_cast<double>(i);
        switch (wf) {
        case WindowFunction::Hamming:
            m_windowCoeffs[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / N);
            break;
        case WindowFunction::Hann:
            m_windowCoeffs[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / N));
            break;
        case WindowFunction::Blackman:
            m_windowCoeffs[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * n / N)
                              + 0.08 * qCos(4.0 * M_PI * n / N);
            break;
        case WindowFunction::FlatTop:
            m_windowCoeffs[i] = 0.21557895
                              - 0.41663158 * qCos(2.0 * M_PI * n / N)
                              + 0.27726316 * qCos(4.0 * M_PI * n / N)
                              - 0.08357895 * qCos(6.0 * M_PI * n / N)
                              + 0.00694737 * qCos(8.0 * M_PI * n / N);
            break;
        case WindowFunction::Kaiser: {
            const double beta = m_params.kaiserBeta;
            const double alpha = N / 2.0;
            double ratio = (n - alpha) / alpha;
            if (ratio < -1.0) ratio = -1.0;
            if (ratio > 1.0) ratio = 1.0;
            double arg = qSqrt(1.0 - ratio * ratio);
            // I0近似
            double sum = 1.0;
            double term = 1.0;
            for (int k = 1; k <= 20; ++k) {
                term *= (beta * arg) / (2.0 * k);
                sum += term * term;
            }
            double denom = 1.0;
            double t = 1.0;
            for (int k = 1; k <= 20; ++k) {
                t *= (beta / 2.0) / k;
                denom += t * t;
            }
            m_windowCoeffs[i] = (denom > 0.0) ? sum / denom : 0.0;
            break;
        }
        }
    }
}

// ============================================================
// Cooley-Tukey radix-2 FFT
// ============================================================

/** @brief 执行radix-2 DIT FFT(原地) */
void SpectrumMonitor::fftRadix2(QVector<std::complex<double>>& data)
{
    const int N = data.size();
    if (N <= 1) return;

    // 位反转
    int bits = 0;
    for (int tmp = N; tmp > 1; tmp >>= 1) ++bits;
    for (int i = 0; i < N; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        if (i < rev) std::swap(data[i], data[rev]);
    }

    // 蝶形运算
    for (int len = 2; len <= N; len <<= 1) {
        const double angle = -2.0 * M_PI / static_cast<double>(len);
        const std::complex<double> wn(qCos(angle), qSin(angle));
        for (int i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                auto t = w * data[i + j + len / 2];
                data[i + j + len / 2] = data[i + j] - t;
                data[i + j] = data[i + j] + t;
                w *= wn;
            }
        }
    }
}

// ============================================================
// dBFS幅度计算
// ============================================================

/** @brief 计算dBFS幅度谱 */
QVector<double> SpectrumMonitor::computeMagnitudes(
    const QVector<std::complex<double>>& fftResult)
{
    const int N = fftResult.size();
    const int halfN = N / 2 + 1;
    QVector<double> mags(halfN);

    // 归一化因子: 窗口能量
    double winEnergy = 0.0;
    for (double w : m_windowCoeffs) winEnergy += w * w;
    if (winEnergy <= 0.0) winEnergy = 1.0;
    const double norm = 2.0 / (static_cast<double>(N) * qSqrt(winEnergy / N));

    for (int i = 0; i < halfN; ++i) {
        double re = fftResult[i].real();
        double im = fftResult[i].imag();
        double mag = qSqrt(re * re + im * im) * norm;
        mags[i] = (mag > 1e-12) ? 20.0 * qLn(mag) / qLn(10.0) : -120.0;
    }
    return mags;
}

// ============================================================
// 峰值保持(指数衰减)
// ============================================================

/** @brief 更新峰值保持 */
void SpectrumMonitor::updatePeakHold(const QVector<double>& magnitudes)
{
    if (m_peakHold.size() != magnitudes.size()) {
        m_peakHold = magnitudes;
        return;
    }
    // 衰减因子: 每帧衰减0.3dB
    const double decay = 0.3;
    for (int i = 0; i < magnitudes.size(); ++i) {
        double decayed = m_peakHold[i] - decay;
        m_peakHold[i] = qMax(magnitudes[i], decayed);
    }
}

// ============================================================
// 瀑布图累积
// ============================================================

/** @brief 追加频谱到瀑布图 */
void SpectrumMonitor::appendSpectrogramRow(const QVector<double>& magnitudes)
{
    m_spectrogram.append(magnitudes);
    while (m_spectrogram.size() > m_spectrogramMaxRows) {
        m_spectrogram.removeFirst();
    }
}
