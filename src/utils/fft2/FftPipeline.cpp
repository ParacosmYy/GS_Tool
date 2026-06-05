/**
 * @file FftPipeline.cpp
 * @brief FFT管道实现
 */

#include "utils/fft2/FftPipeline.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

FftPipeline::FftPipeline(QObject* parent)
    : QObject(parent), m_sampleRate(1000.0),
      m_windowFunc(WindowFunction::Hamming), m_fftSize(256),
      m_timeSum(0.0) {}

void FftPipeline::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void FftPipeline::setWindowFunction(WindowFunction wf) { m_windowFunc = wf; }
void FftPipeline::setFftSize(int size) { m_fftSize = nextPow2(qMax(4, size)); }

FftPipeline::SpectrumResult FftPipeline::forwardFft(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;
    int n = qMin(data.size(), m_fftSize);
    if (n < 2) return result;

    /* 准备输入 */
    QVector<double> input(m_fftSize, 0.0);
    for (int i = 0; i < n; ++i) input[i] = data[i];
    applyWindow(input);

    /* 原地Cooley-Tukey FFT */
    int N = m_fftSize;
    QVector<double> real(N), imag(N);
    for (int i = 0; i < N; ++i) real[i] = input[i];
    imag.fill(0.0);

    /* 位反转 */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= N; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wReal = qCos(ang), wImag = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uReal = real[i + j], uImag = imag[i + j];
                double vReal = curReal * real[i + j + len/2] - curImag * imag[i + j + len/2];
                double vImag = curReal * imag[i + j + len/2] + curImag * real[i + j + len/2];
                real[i + j] = uReal + vReal;
                imag[i + j] = uImag + vImag;
                real[i + j + len/2] = uReal - vReal;
                imag[i + j + len/2] = uImag - vImag;
                double newCurReal = curReal * wReal - curImag * wImag;
                double newCurImag = curReal * wImag + curImag * wReal;
                curReal = newCurReal;
                curImag = newCurImag;
            }
        }
    }

    /* 计算频谱 */
    int halfN = N / 2;
    result.frequencies.resize(halfN);
    result.magnitude.resize(halfN);
    result.phase.resize(halfN);
    result.power.resize(halfN);
    result.totalPower = 0.0;
    result.peakMagnitude = 0.0;

    for (int i = 0; i < halfN; ++i) {
        result.frequencies[i] = static_cast<double>(i) * m_sampleRate / N;
        double mag = qSqrt(real[i] * real[i] + imag[i] * imag[i]) / N;
        double phase = qAtan2(imag[i], real[i]);
        double power = mag * mag;

        result.magnitude[i] = mag;
        result.phase[i] = phase;
        result.power[i] = power;
        result.totalPower += power;

        if (i > 0 && mag > result.peakMagnitude) {
            result.peakMagnitude = mag;
            result.peakFrequency = result.frequencies[i];
        }
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalFrames;
    if (result.peakMagnitude > 0.0) ++m_stats.peakDetections;
    m_timeSum += elapsed;
    m_stats.totalProcessingTimeMs += elapsed;
    m_stats.averageFrameTimeMs = m_stats.totalProcessingTimeMs / m_stats.totalFrames;

    emit spectrumReady(result);
    if (result.peakMagnitude > 0.0) emit peakDetected(result.peakFrequency, result.peakMagnitude);
    return result;
}

QVector<double> FftPipeline::inverseFft(const QVector<double>& real, const QVector<double>& imag)
{
    int N = real.size();
    if (N < 2) return {};

    QVector<double> outReal = real, outImag = imag;

    /* 共轭 */
    for (int i = 0; i < N; ++i) outImag[i] = -outImag[i];

    /* FFT */
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(outReal[i], outReal[j]);
            std::swap(outImag[i], outImag[j]);
        }
    }
    for (int len = 2; len <= N; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uR = outReal[i+j], uI = outImag[i+j];
                double vR = cR*outReal[i+j+len/2] - cI*outImag[i+j+len/2];
                double vI = cR*outImag[i+j+len/2] + cI*outReal[i+j+len/2];
                outReal[i+j] = uR + vR; outImag[i+j] = uI + vI;
                outReal[i+j+len/2] = uR - vR; outImag[i+j+len/2] = uI - vI;
                double nR = cR*wR - cI*wI; cI = cR*wI + cI*wR; cR = nR;
            }
        }
    }

    QVector<double> result(N);
    for (int i = 0; i < N; ++i) result[i] = outReal[i] / N;
    return result;
}

QVector<double> FftPipeline::powerSpectrum(const QVector<double>& data)
{
    SpectrumResult res = forwardFft(data);
    return res.power;
}

QList<FftPipeline::SpectrumResult> FftPipeline::stft(const QVector<double>& data, int hopSize)
{
    QList<SpectrumResult> results;
    if (data.size() < m_fftSize || hopSize < 1) return results;

    for (int start = 0; start + m_fftSize <= data.size(); start += hopSize) {
        QVector<double> frame(m_fftSize);
        for (int i = 0; i < m_fftSize; ++i) frame[i] = data[start + i];
        results.append(forwardFft(frame));
    }
    return results;
}

void FftPipeline::applyWindow(QVector<double>& data)
{
    int n = data.size();
    switch (m_windowFunc) {
    case WindowFunction::None: break;
    case WindowFunction::Hamming:
        for (int i = 0; i < n; ++i) data[i] *= 0.54 - 0.46 * qCos(2.0 * M_PI * i / (n - 1));
        break;
    case WindowFunction::Hanning:
        for (int i = 0; i < n; ++i) data[i] *= 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        break;
    case WindowFunction::Blackman:
        for (int i = 0; i < n; ++i)
            data[i] *= 0.42 - 0.5 * qCos(2.0*M_PI*i/(n-1)) + 0.08 * qCos(4.0*M_PI*i/(n-1));
        break;
    case WindowFunction::FlatTop:
        for (int i = 0; i < n; ++i) {
            double w = 0.21557895 - 0.41663158*qCos(2*M_PI*i/(n-1))
                     + 0.27726316*qCos(4*M_PI*i/(n-1))
                     - 0.08357895*qCos(6*M_PI*i/(n-1))
                     + 0.00694737*qCos(8*M_PI*i/(n-1));
            data[i] *= w;
        }
        break;
    }
}

int FftPipeline::nextPow2(int n) const { int p = 1; while (p < n) p <<= 1; return p; }

void FftPipeline::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
