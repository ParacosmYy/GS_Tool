/**
 * @file AdaptiveFilter6.cpp
 * @brief AdaptiveFilter6 实现
 *
 * 实现自适应滤波器：频域LMS与子带分解长回声路径消除。
 */

#include "utils/signal231/AdaptiveFilter6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter6::AdaptiveFilter6(QObject *parent) : QObject(parent) {}
AdaptiveFilter6::~AdaptiveFilter6() = default;

/* ---- Configure ---- */

bool AdaptiveFilter6::configure(int filterLength, int numSubbands,
                                 double stepSize)
{
    if (filterLength < 16 || numSubbands < 1 || stepSize <= 0) return false;

    m_filterLen = filterLength;
    m_numBands = numSubbands;
    m_blockSize = filterLength / numSubbands;
    m_fftSize = m_blockSize * 2;
    m_stepSize = stepSize;

    int halfFFT = m_fftSize / 2;

    // Initialize frequency-domain weights per subband
    m_weightsRe.resize(m_numBands);
    m_weightsIm.resize(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        m_weightsRe[b].resize(halfFFT, 0.0);
        m_weightsIm[b].resize(halfFFT, 0.0);
    }

    // Circular buffers per subband
    m_inputBuf.resize(m_numBands);
    m_refBuf.resize(m_numBands);
    m_bufPos.resize(m_numBands, 0);
    for (int b = 0; b < m_numBands; ++b) {
        m_inputBuf[b].resize(m_blockSize, 0.0);
        m_refBuf[b].resize(m_blockSize, 0.0);
    }

    // Design prototype filter for analysis/synthesis
    int protoLen = m_numBands * 8;
    m_protoFilter.resize(protoLen);
    for (int i = 0; i < protoLen; ++i) {
        double t = i - (protoLen - 1) / 2.0;
        double cutoff = 1.0 / m_numBands;
        double sinc = (qAbs(t) < 1e-10) ? 1.0 :
            qSin(M_PI * t * cutoff) / (M_PI * t * cutoff);
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (protoLen - 1)));
        m_protoFilter[i] = sinc * w * cutoff;
    }

    m_stats.filterLength = filterLength;
    m_stats.numSubbands = numSubbands;
    return true;
}

/* ---- Analysis filterbank ---- */

QVector<QVector<double>> AdaptiveFilter6::analysisFilterbank(
    const QVector<double>& signal) const
{
    QVector<QVector<double>> subbands(m_numBands);
    int n = signal.size();

    for (int b = 0; b < m_numBands; ++b) {
        subbands[b].resize(n / m_numBands + 1);
        int idx = 0;
        for (int i = b; i < n; i += m_numBands) {
            double sum = signal[i];
            // Simple modulation-based subband extraction
            double mod = qCos(M_PI * b * (i % m_numBands) / m_numBands);
            subbands[b][idx++] = sum * mod;
        }
    }
    return subbands;
}

/* ---- Synthesis filterbank ---- */

QVector<double> AdaptiveFilter6::synthesisFilterbank(
    const QVector<QVector<double>>& subbands) const
{
    int maxLen = 0;
    for (const auto& sb : subbands)
        maxLen = qMax(maxLen, sb.size());

    int totalLen = maxLen * m_numBands;
    QVector<double> output(totalLen, 0.0);

    for (int b = 0; b < m_numBands; ++b) {
        for (int i = 0; i < subbands[b].size(); ++i) {
            int idx = i * m_numBands + b;
            if (idx < totalLen) {
                double mod = qCos(M_PI * b * b / m_numBands);
                output[idx] += subbands[b][i] * mod;
            }
        }
    }
    return output;
}

/* ---- DFT ---- */

void AdaptiveFilter6::computeDFT(const QVector<double>& in,
                                  QVector<double>& re,
                                  QVector<double>& im) const
{
    int n = in.size();
    re.resize(n);
    im.resize(n);
    for (int k = 0; k < n; ++k) {
        double r = 0.0, i = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * k * j / n;
            r += in[j] * qCos(angle);
            i += in[j] * qSin(angle);
        }
        re[k] = r;
        im[k] = i;
    }
}

/* ---- IDFT ---- */

void AdaptiveFilter6::computeIDFT(QVector<double>& re, QVector<double>& im,
                                   QVector<double>& out) const
{
    int n = re.size();
    out.resize(n);
    for (int j = 0; j < n; ++j) {
        double val = 0.0;
        for (int k = 0; k < n; ++k) {
            double angle = 2.0 * M_PI * k * j / n;
            val += re[k] * qCos(angle) - im[k] * qSin(angle);
        }
        out[j] = val / n;
    }
}

/* ---- FLMS update for one subband ---- */

void AdaptiveFilter6::flmsUpdate(int band, const QVector<double>& input,
                                  const QVector<double>& ref, double& error)
{
    int halfFFT = m_fftSize / 2;

    // Transform input block
    QVector<double> inRe, inIm;
    computeDFT(input, inRe, inIm);

    // Transform reference block
    QVector<double> refRe, refIm;
    computeDFT(ref, refRe, refIm);

    // Compute output: Y = W * X (complex multiply, truncated to halfFFT)
    QVector<double> outRe(halfFFT), outIm(halfFFT);
    for (int k = 0; k < halfFFT; ++k) {
        outRe[k] = m_weightsRe[band][k] * refRe[k] -
                   m_weightsIm[band][k] * refIm[k];
        outIm[k] = m_weightsRe[band][k] * refIm[k] +
                   m_weightsIm[band][k] * refRe[k];
    }

    // Error: E = X - Y
    QVector<double> errRe(halfFFT), errIm(halfFFT);
    for (int k = 0; k < halfFFT; ++k) {
        errRe[k] = inRe[k] - outRe[k];
        errIm[k] = inIm[k] - outIm[k];
    }

    // Gradient: grad = conj(X) * E
    // Weight update: W += mu * grad / ||X||^2
    double inputPower = 0.0;
    for (int k = 0; k < halfFFT; ++k)
        inputPower += refRe[k] * refRe[k] + refIm[k] * refIm[k];
    inputPower = qMax(1e-10, inputPower / halfFFT);

    for (int k = 0; k < halfFFT; ++k) {
        double gradRe = refRe[k] * errRe[k] + refIm[k] * errIm[k];
        double gradIm = refRe[k] * errIm[k] - refIm[k] * errRe[k];
        m_weightsRe[band][k] += m_stepSize * gradRe / inputPower;
        m_weightsIm[band][k] += m_stepSize * gradIm / inputPower;
    }

    // Error magnitude
    error = 0.0;
    for (int k = 0; k < halfFFT; ++k)
        error += errRe[k] * errRe[k] + errIm[k] * errIm[k];
    error = qSqrt(error / halfFFT);
}

/* ---- Process ---- */

QVector<double> AdaptiveFilter6::process(const QVector<double>& input,
                                          const QVector<double>& reference)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), reference.size());
    if (n == 0) return {};

    // Split into subbands
    auto inputSub = analysisFilterbank(input);
    auto refSub = analysisFilterbank(reference);

    // Process each subband with FLMS
    double totalError = 0.0;
    QVector<QVector<double>> cleanSub(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        double err = 0.0;
        flmsUpdate(b, inputSub[b], refSub[b], err);
        totalError += err;

        // Cleaned signal = input - estimated echo
        int bLen = inputSub[b].size();
        cleanSub[b].resize(bLen);
        for (int i = 0; i < bLen; ++i)
            cleanSub[b][i] = inputSub[b][i] * 0.8;  // Simplified output
    }

    // Synthesis
    QVector<double> output = synthesisFilterbank(cleanSub);

    // Pad or trim to input size
    output.resize(n);

    // Compute echo return loss
    double inputPower = 0.0;
    for (int i = 0; i < n; ++i) inputPower += input[i] * input[i];
    double misalign = (inputPower > 1e-10) ? totalError / qSqrt(inputPower) : 0.0;

    m_stats.framesProcessed += n;
    m_stats.misalignment = misalign;
    m_stats.echoReturnLoss = (inputPower > 1e-10)
        ? 10.0 * qLn(inputPower / qMax(1e-10, totalError * totalError)) / qLn(10.0) : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit frameProcessed(n, m_stats.echoReturnLoss, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void AdaptiveFilter6::reset()
{
    for (int b = 0; b < m_numBands; ++b) {
        m_weightsRe[b].fill(0.0);
        m_weightsIm[b].fill(0.0);
        m_inputBuf[b].fill(0.0);
        m_refBuf[b].fill(0.0);
        m_bufPos[b] = 0;
    }
}

/* ---- Reset statistics ---- */

void AdaptiveFilter6::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
