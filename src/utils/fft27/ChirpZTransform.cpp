/**
 * @file ChirpZTransform.cpp
 * @brief Chirp-Z变换实现
 */

#include "utils/fft27/ChirpZTransform.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

ChirpZTransform::ChirpZTransform(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_startFreq(0.0)
    , m_endFreq(22050.0)
    , m_outputPoints(512)
    , m_inputLength(0)
    , m_timeSum(0.0)
{
}

void ChirpZTransform::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

void ChirpZTransform::setFrequencyRange(double startHz, double endHz)
{
    m_startFreq = qMax(0.0, startHz);
    m_endFreq = qMax(m_startFreq + 1.0, endHz);
}

void ChirpZTransform::setOutputPoints(int m)
{
    m_outputPoints = qMax(1, m);
}

void ChirpZTransform::setInputLength(int n)
{
    m_inputLength = qMax(0, n);
}

ChirpZTransform::CztResult ChirpZTransform::transform(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    CztResult result;
    int n = data.size();
    if (n == 0) return result;
    m_inputLength = n;

    int m = m_outputPoints;
    QVector<double> outReal, outImag;
    bluesteinFft(data, outReal, outImag);

    /* 生成频率轴和幅度/相位 */
    result.frequencies.reserve(m);
    result.magnitude.reserve(m);
    result.phase.reserve(m);
    double freqStep = (m_endFreq - m_startFreq) / qMax(m - 1, 1);
    double peakMag = 0.0;
    double peakFreq = 0.0;

    for (int k = 0; k < m && k < outReal.size(); ++k) {
        double freq = m_startFreq + k * freqStep;
        result.frequencies.append(freq);
        double mag = qSqrt(outReal[k] * outReal[k] + outImag[k] * outImag[k]);
        double ph = qAtan2(outImag[k], outReal[k]);
        result.magnitude.append(mag);
        result.phase.append(ph);
        if (mag > peakMag) {
            peakMag = mag;
            peakFreq = freq;
        }
    }
    result.peakFrequency = peakFreq;
    result.peakMagnitude = peakMag;

    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformComplete(n, m, peakFreq);
    if (peakMag > 0.0) emit peakDetected(peakFreq, peakMag);
    return result;
}

ChirpZTransform::CztResult ChirpZTransform::zoomFft(const QVector<double>& data,
    double centerHz, double bandwidthHz, int resolution)
{
    double startHz = centerHz - bandwidthHz / 2.0;
    double endHz = centerHz + bandwidthHz / 2.0;
    setFrequencyRange(qMax(0.0, startHz), endHz);
    setOutputPoints(resolution);
    return transform(data);
}

QVector<double> ChirpZTransform::inverseTransform(const QVector<double>& magnitude,
    const QVector<double>& phase)
{
    if (magnitude.size() != phase.size()) return {};

    int m = magnitude.size();
    /* 重建复数频域数据 */
    QVector<double> real(m), imag(m);
    for (int i = 0; i < m; ++i) {
        real[i] = magnitude[i] * qCos(phase[i]);
        imag[i] = magnitude[i] * qSin(phase[i]);
    }
    /* 反FFT */
    fft(real, imag, true);
    return real;
}

void ChirpZTransform::bluesteinFft(const QVector<double>& input,
    QVector<double>& outReal, QVector<double>& outImag)
{
    int n = input.size();
    int m = m_outputPoints;
    int L = nextPow2(n + m - 1);

    /* 生成chirp信号 y[k] = exp(-j*pi*k^2/M) */
    QVector<double> chirpReal(L, 0.0), chirpImag(L, 0.0);
    for (int k = 0; k < n + m - 1; ++k) {
        double angle = -M_PI * k * k / m;
        chirpReal[k] = qCos(angle);
        chirpImag[k] = qSin(angle);
    }

    /* 输入与共轭chirp相乘 */
    QVector<double> aReal(L, 0.0), aImag(L, 0.0);
    for (int k = 0; k < n; ++k) {
        aReal[k] = input[k] * chirpReal[k];
        aImag[k] = input[k] * (-chirpImag[k]);
    }

    /* chirp自身的FFT */
    QVector<double> bReal(L, 0.0), bImag(L, 0.0);
    for (int k = 0; k < L; ++k) {
        int idx = (k == 0) ? 0 : L - k;
        if (idx < n + m - 1) {
            bReal[k] = chirpReal[idx];
            bImag[k] = -chirpImag[idx];
        }
    }

    /* FFT(a) * FFT(b) */
    fft(aReal, aImag, false);
    fft(bReal, bImag, false);
    for (int k = 0; k < L; ++k) {
        double tr = aReal[k] * bReal[k] - aImag[k] * bImag[k];
        double ti = aReal[k] * bImag[k] + aImag[k] * bReal[k];
        aReal[k] = tr;
        aImag[k] = ti;
    }

    /* IFFT */
    fft(aReal, aImag, true);

    /* 乘以共轭chirp得到最终结果 */
    outReal.resize(m);
    outImag.resize(m);
    for (int k = 0; k < m; ++k) {
        outReal[k] = aReal[k] * chirpReal[k] - aImag[k] * (-chirpImag[k]);
        outImag[k] = aReal[k] * (-chirpImag[k]) + aImag[k] * chirpReal[k];
    }
}

void ChirpZTransform::fft(QVector<double>& real, QVector<double>& imag, bool inverse)
{
    int n = real.size();
    if (n <= 1) return;

    /* 位逆序置换 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? -1.0 : 1.0);
        double wReal = qCos(angle);
        double wImag = qSin(angle);
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

    if (inverse) {
        for (int i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

int ChirpZTransform::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

void ChirpZTransform::generateChirp(int len, QVector<double>& real,
    QVector<double>& imag)
{
    real.resize(len);
    imag.resize(len);
    for (int k = 0; k < len; ++k) {
        double angle = -M_PI * k * k / m_outputPoints;
        real[k] = qCos(angle);
        imag[k] = qSin(angle);
    }
}

void ChirpZTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
