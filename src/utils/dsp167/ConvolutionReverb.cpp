/**
 * @file ConvolutionReverb.cpp
 * @brief ConvolutionReverb 实现
 *
 * 实现分块卷积混响：Overlap-Save方法、分块FFT、实时音频流处理。
 */

#include "utils/dsp167/ConvolutionReverb.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

ConvolutionReverb::ConvolutionReverb(QObject* parent)
    : QObject(parent)
{
}

ConvolutionReverb::~ConvolutionReverb() = default;

void ConvolutionReverb::setBlockSize(int size)
{
    /* Ensure power of 2 */
    int sz = 1;
    while (sz < size) sz <<= 1;
    m_blockSize = qMax(64, sz);
}

void ConvolutionReverb::setWetDryMix(double wet)
{
    m_wetDryMix = qBound(0.0, wet, 1.0);
}

int ConvolutionReverb::bitReverse(int x, int log2n)
{
    int result = 0;
    for (int i = 0; i < log2n; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

void ConvolutionReverb::fft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, log2n);
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int step = 1; step < n; step <<= 1) {
        double angle = M_PI / step;
        double wReal = qCos(angle);
        double wImag = -qSin(angle);
        for (int i = 0; i < n; i += (step << 1)) {
            double curReal = 1.0, curImag = 0.0;
            for (int j = i; j < i + step; ++j) {
                double tReal = curReal * real[j + step] - curImag * imag[j + step];
                double tImag = curReal * imag[j + step] + curImag * real[j + step];
                real[j + step] = real[j] - tReal;
                imag[j + step] = imag[j] - tImag;
                real[j] += tReal;
                imag[j] += tImag;
                double nr = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = nr;
            }
        }
    }
}

void ConvolutionReverb::ifft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
}

void ConvolutionReverb::partitionIR()
{
    if (m_irRealParts.isEmpty()) return;

    int irLen = m_irRealParts[0].size();
    m_fftSize = m_blockSize * 2;
    m_partitionCount = (irLen + m_blockSize - 1) / m_blockSize;

    /* Zero-pad IR partitions to fftSize */
    m_irRealParts.clear();
    m_irImagParts.clear();

    int pos = 0;
    /* Store original IR for partitioning */
    QVector<double> irData = m_overlapTail;
    m_overlapTail.clear();

    for (int p = 0; p < m_partitionCount; ++p) {
        QVector<double> rPart(m_fftSize, 0.0);
        QVector<double> iPart(m_fftSize, 0.0);
        for (int i = 0; i < m_blockSize && pos + i < irData.size(); ++i)
            rPart[i] = irData[pos + i];
        pos += m_blockSize;
        fft(rPart, iPart);
        m_irRealParts.append(rPart);
        m_irImagParts.append(iPart);
    }

    /* Initialize history ring */
    m_historyReal.resize(m_partitionCount);
    m_historyImag.resize(m_partitionCount);
    for (int p = 0; p < m_partitionCount; ++p) {
        m_historyReal[p].resize(m_fftSize, 0.0);
        m_historyImag[p].resize(m_fftSize, 0.0);
    }
}

void ConvolutionReverb::loadImpulseResponse(const QVector<double>& ir)
{
    m_overlapTail = ir;
    partitionIR();
    m_stats.irLength = ir.size();
    emit impulseResponseLoaded(ir.size());
}

QVector<double> ConvolutionReverb::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int bs = m_blockSize;
    if (input.size() != bs || m_irRealParts.isEmpty()) {
        emit blockProcessed(bs);
        return QVector<double>(bs, 0.0);
    }

    /* FFT of zero-padded input block */
    QVector<double> inReal(m_fftSize, 0.0);
    QVector<double> inImag(m_fftSize, 0.0);
    for (int i = 0; i < bs; ++i) inReal[i] = input[i];
    fft(inReal, inImag);

    /* Shift history and insert new transform */
    for (int p = m_partitionCount - 1; p > 0; --p) {
        m_historyReal[p] = m_historyReal[p - 1];
        m_historyImag[p] = m_historyImag[p - 1];
    }
    m_historyReal[0] = inReal;
    m_historyImag[0] = inImag;

    /* Overlap-save: multiply and accumulate in frequency domain */
    QVector<double> outReal(m_fftSize, 0.0);
    QVector<double> outImag(m_fftSize, 0.0);
    for (int p = 0; p < m_partitionCount; ++p) {
        for (int i = 0; i < m_fftSize; ++i) {
            outReal[i] += m_historyReal[p][i] * m_irRealParts[p][i]
                        - m_historyImag[p][i] * m_irImagParts[p][i];
            outImag[i] += m_historyReal[p][i] * m_irImagParts[p][i]
                        + m_historyImag[p][i] * m_irRealParts[p][i];
        }
    }

    /* IFFT to get time-domain output */
    ifft(outReal, outImag);

    /* Extract valid samples (discard first blockSize) */
    QVector<double> output(bs);
    for (int i = 0; i < bs; ++i)
        output[i] = (1.0 - m_wetDryMix) * input[i]
                   + m_wetDryMix * outReal[i + bs];

    m_stats.totalBlocks++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalBlocks > 0)
        ? m_timeSum / m_stats.totalBlocks : 0.0;

    emit blockProcessed(bs);
    return output;
}

void ConvolutionReverb::reset()
{
    for (int p = 0; p < m_partitionCount; ++p) {
        m_historyReal[p].fill(0.0);
        m_historyImag[p].fill(0.0);
    }
}

void ConvolutionReverb::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
