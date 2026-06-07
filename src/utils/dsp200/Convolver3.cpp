/**
 * @file Convolver3.cpp
 * @brief Convolver3 实现
 *
 * 实现分区卷积：均匀FFT分区、零延迟频域处理、长脉冲响应实时卷积。
 */

#include "utils/dsp200/Convolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Convolver3::Convolver3(QObject *parent) : QObject(parent) {}
Convolver3::~Convolver3() = default;

/* ---- Configuration ---- */

void Convolver3::setBlockSize(int size)
{
    m_blockSize = qMax(4, size);
    m_fftSize = m_blockSize * 2;
    precomputePartitions();
}

void Convolver3::setImpulseResponse(const QVector<double>& ir)
{
    m_ir = ir;
    precomputePartitions();
}

/* ---- FFT (Cooley-Tukey radix-2) ---- */

void Convolver3::bitReverse(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            qSwap(re[i], re[j]); qSwap(im[i], im[j]);
        }
    }
}

void Convolver3::fft(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    bitReverse(re, im);
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * re[i + j + len / 2] - curIm * im[i + j + len / 2];
                double tIm = curRe * im[i + j + len / 2] + curIm * re[i + j + len / 2];
                re[i + j + len / 2] = re[i + j] - tRe;
                im[i + j + len / 2] = im[i + j] - tIm;
                re[i + j] += tRe;
                im[i + j] += tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

void Convolver3::ifft(QVector<double>& re, QVector<double>& im)
{
    int n = re.size();
    for (int i = 0; i < n; ++i) im[i] = -im[i];
    fft(re, im);
    double inv = 1.0 / n;
    for (int i = 0; i < n; ++i) { re[i] *= inv; im[i] *= -inv; }
}

/* ---- Complex multiply ---- */

void Convolver3::complexMultiply(double& ar, double& ai, double br, double bi)
{
    double tr = ar * br - ai * bi;
    double ti = ar * bi + ai * br;
    ar = tr; ai = ti;
}

/* ---- Precompute IR partitions ---- */

void Convolver3::precomputePartitions()
{
    if (m_ir.isEmpty()) { m_numPartitions = 0; return; }

    m_numPartitions = (m_ir.size() + m_blockSize - 1) / m_blockSize;
    m_irFreqRe.resize(m_numPartitions);
    m_irFreqIm.resize(m_numPartitions);

    for (int p = 0; p < m_numPartitions; ++p) {
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);
        int offset = p * m_blockSize;
        int len = qMin(m_blockSize, m_ir.size() - offset);
        for (int i = 0; i < len; ++i) re[i] = m_ir[offset + i];
        fft(re, im);
        m_irFreqRe[p] = re;
        m_irFreqIm[p] = im;
    }

    // Initialize frequency domain circular buffer
    m_freqBufRe.resize(m_numPartitions);
    m_freqBufIm.resize(m_numPartitions);
    for (int p = 0; p < m_numPartitions; ++p) {
        m_freqBufRe[p].resize(m_fftSize, 0.0);
        m_freqBufIm[p].resize(m_fftSize, 0.0);
    }
    m_bufHead = 0;
    m_overlapBuf.resize(m_blockSize, 0.0);
}

/* ---- Process one block (overlap-save) ---- */

QVector<double> Convolver3::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    // FFT input block with zero-padding
    QVector<double> inRe(m_fftSize, 0.0), inIm(m_fftSize, 0.0);
    for (int i = 0; i < qMin(input.size(), m_blockSize); ++i) inRe[i] = input[i];
    fft(inRe, inIm);

    // Store in circular buffer
    m_freqBufRe[m_bufHead] = inRe;
    m_freqBufIm[m_bufHead] = inIm;

    // Accumulate: sum over partitions of (input_{p} * IR_p)
    QVector<double> accRe(m_fftSize, 0.0), accIm(m_fftSize, 0.0);
    for (int p = 0; p < m_numPartitions; ++p) {
        int idx = (m_bufHead - p + m_numPartitions) % m_numPartitions;
        for (int i = 0; i < m_fftSize; ++i) {
            double re = m_freqBufRe[idx][i], im = m_freqBufIm[idx][i];
            complexMultiply(re, im, m_irFreqRe[p][i], m_irFreqIm[p][i]);
            accRe[i] += re; accIm[i] += im;
        }
    }

    // IFFT and extract valid part (overlap-save)
    ifft(accRe, accIm);

    // Add overlap from previous block
    QVector<double> output(m_blockSize);
    for (int i = 0; i < m_blockSize; ++i)
        output[i] = accRe[i + m_blockSize]; // Valid part (second half)

    m_bufHead = (m_bufHead + 1) % m_numPartitions;

    m_stats.totalBlocks++;
    m_stats.blockSize = m_blockSize;
    m_stats.numPartitions = m_numPartitions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit blockProcessed(static_cast<int>(m_stats.totalBlocks), timer.elapsed());
    return output;
}

/* ---- Full convolution ---- */

QVector<double> Convolver3::convolve(const QVector<double>& input) const
{
    int n = input.size();
    int irLen = m_ir.size();
    int outLen = n + irLen - 1;
    if (outLen <= 0) return {};

    // Direct convolution
    QVector<double> output(outLen, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < irLen; ++j)
            output[i + j] += input[i] * m_ir[j];
    return output;
}

/* ---- Reset ---- */

void Convolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (int p = 0; p < m_numPartitions; ++p) {
        m_freqBufRe[p].fill(0.0);
        m_freqBufIm[p].fill(0.0);
    }
    m_overlapBuf.fill(0.0);
    m_bufHead = 0;
}
