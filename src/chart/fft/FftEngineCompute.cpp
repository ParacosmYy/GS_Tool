/**
 * @file FftEngineCompute.cpp
 * @brief FFT频谱计算引擎 — 主计算接口与Cooley-Tukey蝶形运算实现
 *
 * 从 FftEngine.cpp 拆分而来，包含:
 *   - compute(): 主频谱计算接口(加窗→FFT→单边幅度谱→峰值检测)
 *   - fftRadix2(): Cooley-Tukey radix-2 DIT原地FFT
 *
 * 构造/工具方法见 FftEngine.cpp。
 * 窗函数(applyWindow)/幅度谱(magnitudeSpectrum)/统计接口见 FftEngineWindow.cpp。
 */

#include "chart/fft/FftEngine.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

/** @brief 执行FFT频谱计算: 加窗→FFT→单边幅度谱 @param timeData 时域采样点(x=序号,y=采样值) @param sampleRate 采样率(Hz) @param window 窗函数类型 @param fftSize FFT长度(2的幂,0=自动) @return 频谱数据(x=频率Hz,y=幅度)，长度为fftSize/2 */
QVector<QPointF> FftEngine::compute(const QVector<QPointF>& timeData,
                                    double sampleRate,
                                    WindowType window,
                                    int fftSize)
{
    /* 无数据时返回空频谱 */
    if (timeData.isEmpty() || sampleRate <= 0.0) {
        ++m_errorCount;
        return {};
    }

    /* 跟踪窗函数类型变更 */
    static WindowType lastWindow = WindowType::Hanning;
    if (window != lastWindow) {
        ++m_totalWindowTypeChanges;
        lastWindow = window;
    }

    /* 确定FFT长度: 用户指定 or 自动取nextPowerOf2 */
    const int N = (fftSize > 0) ? nextPowerOf2(fftSize)
                                : nextPowerOf2(timeData.size());

    /* 统计: FFT长度变更检测 */
    if (m_lastFftSize != 0 && N != m_lastFftSize) {
        ++m_totalSizeChanges;
    }
    m_lastFftSize = N;

    /* 统计: 零填充检测(输入数据不足FFT长度) */
    if (timeData.size() < N) {
        ++m_totalZeroPaddingCount;
    }

    /* 计时: 测量FFT计算耗时 */
    QElapsedTimer computeTimer;
    computeTimer.start();

    /* 构造复数序列，从时域数据的Y值提取 */
    QVector<std::complex<double>> data;
    data.reserve(N);
    for (int i = 0; i < N; ++i) {
        if (i < timeData.size()) {
            data.append(std::complex<double>(timeData[i].y(), 0.0));
        } else {
            data.append(std::complex<double>(0.0, 0.0));
        }
    }

    /* 1. 应用窗函数 */
    applyWindow(data, window);

    /* 2. 执行FFT（原地） */
    fftRadix2(data);

    /* 3. 计算单边幅度谱 */
    QVector<QPointF> spectrum = magnitudeSpectrum(data, sampleRate);

    /* 4. 查找基频（幅度最大处对应的频率） */
    double fundamentalFreq = 0.0;
    double maxMag = 0.0;
    for (const auto& pt : spectrum) {
        if (pt.y() > maxMag) {
            maxMag = pt.y();
            fundamentalFreq = pt.x();
        }
    }

    /* 更新统计计数器 */
    ++m_totalTransforms;
    ++m_totalTransformsExecuted;
    m_totalSamplesProcessed += static_cast<quint64>(timeData.size());
    quint64 nSample = static_cast<quint64>(timeData.size());
    if (nSample > m_maxSampleSize) {
        m_maxSampleSize = nSample;
    }
    if (maxMag > 0.0) {
        ++m_totalPeakFrequenciesDetected;
    }

    /* 统计: 累计计算耗时 */
    m_totalComputeTimeUs += static_cast<quint64>(computeTimer.nsecsElapsed() / 1000);
    ++m_totalComputeCount;

    emit spectrumComputed(spectrum, fundamentalFreq);
    return spectrum;
}

/** @brief 执行Cooley-Tukey radix-2 DIT FFT原地计算 @param data 输入/输出复数序列(长度必须为2的幂) */
void FftEngine::fftRadix2(QVector<std::complex<double>>& data)
{
    const int N = data.size();
    if (N <= 1) {
        return;
    }

    const int stages = log2Int(N);

    /* 步骤1: 位反转置换 */
    for (int i = 0; i < N; ++i) {
        int j = bitReverse(i, stages);
        if (j > i) {
            std::swap(data[i], data[j]);
        }
    }

    /* 步骤2: 蝶形运算 */
    for (int stage = 1; stage <= stages; ++stage) {
        int span = (1 << stage);
        int halfSpan = span >> 1;

        double angleStep = -2.0 * M_PI / span;
        double wReal = qCos(angleStep);
        double wImag = qSin(angleStep);

        for (int base = 0; base < N; base += span) {
            double curReal = 1.0;
            double curImag = 0.0;

            for (int k = 0; k < halfSpan; ++k) {
                int top = base + k;
                int bot = top + halfSpan;

                double tReal = curReal * data[bot].real() - curImag * data[bot].imag();
                double tImag = curReal * data[bot].imag() + curImag * data[bot].real();

                data[bot] = std::complex<double>(
                    data[top].real() - tReal,
                    data[top].imag() - tImag);
                data[top] = std::complex<double>(
                    data[top].real() + tReal,
                    data[top].imag() + tImag);

                double newReal = curReal * wReal - curImag * wImag;
                double newImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
                curImag = newImag;
            }
        }
    }
}
