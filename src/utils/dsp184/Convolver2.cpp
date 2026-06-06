/**
 * @file Convolver2.cpp
 * @brief Convolver2 实现
 *
 * 实现快速卷积：重叠相加/保留FFT、分段IR处理、实时流式卷积。
 */

#include "utils/dsp184/Convolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Convolver2::Convolver2(QObject *parent) : QObject(parent) {}
Convolver2::~Convolver2() = default;

/* ---- Configuration ---- */

void Convolver2::setMethod(Method method) { m_method = method; }

void Convolver2::setBlockSize(int size)
{
    m_blockSize = qMax(64, size);
    partitionIR();
}

void Convolver2::setImpulseResponse(const QVector<double>& ir)
{
    m_ir = ir;
    partitionIR();
}

/* ---- Helpers ---- */

int Convolver2::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

void Convolver2::complexMul(double ar, double ai, double br, double bi,
                              double& cr, double& ci) const
{
    cr = ar * br - ai * bi;
    ci = ar * bi + ai * br;
}

/* ---- FFT ---- */

void Convolver2::fft(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;
    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len *= 2) {
        double ang = sign * 2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR, tI;
                complexMul(cR, cI, re[o], im[o], tR, tI);
                re[o] = re[e] - tR; im[o] = im[e] - tI;
                re[e] += tR; im[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nR;
            }
        }
    }
    if (inverse) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Partition IR ---- */

void Convolver2::partitionIR()
{
    if (m_ir.isEmpty() || m_blockSize <= 0) return;

    m_fftSize = nextPow2(m_blockSize * 2);
    m_numPartitions = (m_ir.size() + m_blockSize - 1) / m_blockSize;

    m_irPartRe.resize(m_numPartitions);
    m_irPartIm.resize(m_numPartitions);

    for (int p = 0; p < m_numPartitions; ++p) {
        m_irPartRe[p].resize(m_fftSize, 0.0);
        m_irPartIm[p].resize(m_fftSize, 0.0);

        int start = p * m_blockSize;
        int len = qMin(m_blockSize, m_ir.size() - start);
        for (int i = 0; i < len; ++i)
            m_irPartRe[p][i] = m_ir[start + i];

        fft(m_irPartRe[p], m_irPartIm[p], false);
    }

    m_overlapBuffer.resize(m_fftSize, 0.0);
    m_tail.resize(m_fftSize, 0.0);
}

/* ---- Overlap-Add ---- */

QVector<double> Convolver2::overlapAdd(const QVector<double>& input)
{
    int N = m_fftSize;
    QVector<double> inRe(N, 0.0), inIm(N, 0.0);

    int len = qMin(input.size(), m_blockSize);
    for (int i = 0; i < len; ++i) inRe[i] = input[i];

    fft(inRe, inIm, false);

    QVector<double> outRe(N, 0.0), outIm(N, 0.0);

    // Multiply with each partition and accumulate
    for (int p = 0; p < m_numPartitions; ++p) {
        QVector<double> prodRe(N), prodIm(N);
        for (int i = 0; i < N; ++i)
            complexMul(inRe[i], inIm[i],
                        m_irPartRe[p][i], m_irPartIm[p][i],
                        prodRe[i], prodIm[i]);
        for (int i = 0; i < N; ++i) {
            outRe[i] += prodRe[i];
            outIm[i] += prodIm[i];
        }
    }

    fft(outRe, outIm, true);

    // Add overlap from previous block
    for (int i = 0; i < N; ++i) outRe[i] += m_overlapBuffer.value(i, 0.0);

    // Store overlap for next block
    m_overlapBuffer.resize(N);
    for (int i = m_blockSize; i < N; ++i) m_overlapBuffer[i] = outRe[i];

    return outRe.mid(0, m_blockSize);
}

/* ---- Overlap-Save ---- */

QVector<double> Convolver2::overlapSave(const QVector<double>& input)
{
    int N = m_fftSize;
    QVector<double> inRe(N, 0.0), inIm(N, 0.0);

    // Prepend tail from previous block
    int tailLen = N - m_blockSize;
    for (int i = 0; i < tailLen && i < m_tail.size(); ++i)
        inRe[i] = m_tail[i];

    int len = qMin(input.size(), m_blockSize);
    for (int i = 0; i < len; ++i)
        inRe[tailLen + i] = input[i];

    // Store current input as next tail
    m_tail.resize(len);
    for (int i = 0; i < len; ++i) m_tail[i] = input[i];

    fft(inRe, inIm, false);

    QVector<double> outRe(N, 0.0), outIm(N, 0.0);
    for (int p = 0; p < m_numPartitions; ++p) {
        QVector<double> prodRe(N), prodIm(N);
        for (int i = 0; i < N; ++i)
            complexMul(inRe[i], inIm[i],
                        m_irPartRe[p][i], m_irPartIm[p][i],
                        prodRe[i], prodIm[i]);
        for (int i = 0; i < N; ++i) {
            outRe[i] += prodRe[i];
            outIm[i] += prodIm[i];
        }
    }

    fft(outRe, outIm, true);

    // Discard first (N - blockSize) samples (corrupted by circular conv)
    return outRe.mid(N - m_blockSize, m_blockSize);
}

/* ---- Process block ---- */

QVector<double> Convolver2::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (m_method == Method::OverlapAdd)
        result = overlapAdd(input);
    else
        result = overlapSave(input);

    m_stats.totalBlocks++;
    m_stats.blockSize = m_blockSize;
    m_stats.irLength = m_ir.size();
    m_stats.fftSize = m_fftSize;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit blockProcessed(m_blockSize, timer.elapsed());
    return result;
}

/* ---- Full convolution ---- */

QVector<double> Convolver2::convolve(const QVector<double>& input,
                                       const QVector<double>& ir) const
{
    int N = nextPow2(input.size() + ir.size() - 1);
    QVector<double> aRe(N, 0.0), aIm(N, 0.0);
    QVector<double> bRe(N, 0.0), bIm(N, 0.0);

    for (int i = 0; i < input.size(); ++i) aRe[i] = input[i];
    for (int i = 0; i < ir.size(); ++i) bRe[i] = ir[i];

    // Cast away const for fft (logic is const)
    Convolver2* self = const_cast<Convolver2*>(this);
    self->fft(aRe, aIm, false);
    self->fft(bRe, bIm, false);

    for (int i = 0; i < N; ++i)
        self->complexMul(aRe[i], aIm[i], bRe[i], bIm[i], aRe[i], aIm[i]);

    self->fft(aRe, aIm, true);

    int outLen = input.size() + ir.size() - 1;
    return aRe.mid(0, outLen);
}

/* ---- Reset ---- */

void Convolver2::reset()
{
    m_overlapBuffer.fill(0.0);
    m_tail.fill(0.0);
}

void Convolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
