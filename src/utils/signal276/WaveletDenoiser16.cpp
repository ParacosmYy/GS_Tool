/**
 * @file WaveletDenoiser16.cpp
 * @brief WaveletDenoiser16 实现
 *
 * 实现小波去噪器：贝叶斯收缩与尺度间依赖建模信号保留噪声去除。
 */

#include "utils/signal276/WaveletDenoiser16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser16::WaveletDenoiser16(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser16::~WaveletDenoiser16() = default;

/* ---- Configuration ---- */

void WaveletDenoiser16::setLevels(int levels) { m_levels = qBound(1, levels, 15); }
void WaveletDenoiser16::setWaveletType(int type) { m_waveletType = qBound(0, type, 2); }
void WaveletDenoiser16::setNoiseStd(double sigma) { m_noiseStd = qMax(1e-10, sigma); }

/* ---- Wavelet filter coefficients ---- */

void WaveletDenoiser16::getFilterCoeffs(QVector<double>& loDec, QVector<double>& hiDec,
                                          QVector<double>& loRec, QVector<double>& hiRec) const
{
    if (m_waveletType == 1) {
        // DB2 (Daubechies 4-tap)
        loDec = {0.4829629131, 0.8365163037, 0.2241438680, -0.1294095226};
        hiDec = {-0.1294095226, -0.2241438680, 0.8365163037, -0.4829629131};
        loRec = {-0.1294095226, 0.2241438680, 0.8365163037, 0.4829629131};
        hiRec = {-0.4829629131, 0.8365163037, -0.2241438680, -0.1294095226};
    } else if (m_waveletType == 2) {
        // DB4 (Daubechies 8-tap)
        loDec = {-0.0105974017, 0.0328830117, 0.0308413818, -0.1870348117,
                 -0.0279837694, 0.6308807679, 0.7148465706, 0.2303778133};
        hiDec = {-0.2303778133, 0.7148465706, -0.6308807679, -0.0279837694,
                  0.1870348117, 0.0308413818, -0.0328830117, -0.0105974017};
        loRec = {0.2303778133, 0.7148465706, 0.6308807679, -0.0279837694,
                 -0.1870348117, 0.0308413818, 0.0328830117, -0.0105974017};
        hiRec = {-0.0105974017, -0.0328830117, 0.0308413818, 0.1870348117,
                 -0.0279837694, -0.6308807679, 0.7148465706, -0.2303778133};
    } else {
        // Haar (2-tap)
        loDec = {0.7071067812, 0.7071067812};
        hiDec = {-0.7071067812, 0.7071067812};
        loRec = {0.7071067812, 0.7071067812};
        hiRec = {0.7071067812, -0.7071067812};
    }
}

/* ---- Forward DWT ---- */

void WaveletDenoiser16::forwardDWT(const QVector<double>& signal)
{
    m_detailCoeffs.clear();
    m_detailCoeffs.resize(m_levels);

    QVector<double> current = signal;
    QVector<double> loDec, hiDec, loRec, hiRec;
    getFilterCoeffs(loDec, hiDec, loRec, hiRec);
    int fLen = loDec.size();

    for (int lev = 0; lev < m_levels; ++lev) {
        int n = current.size();
        if (n < fLen) break;
        int half = n / 2;

        QVector<double> approx(half, 0.0);
        QVector<double> detail(half, 0.0);

        for (int i = 0; i < half; ++i) {
            double aSum = 0.0, dSum = 0.0;
            for (int k = 0; k < fLen; ++k) {
                int idx = (2 * i + k) % n;
                aSum += loDec[k] * current[idx];
                dSum += hiDec[k] * current[idx];
            }
            approx[i] = aSum;
            detail[i] = dSum;
        }

        m_detailCoeffs[lev] = detail;
        current = approx;
    }
    m_approxCoeffs = current;
}

/* ---- Inverse DWT ---- */

QVector<double> WaveletDenoiser16::inverseDWT()
{
    QVector<double> loDec, hiDec, loRec, hiRec;
    getFilterCoeffs(loDec, hiDec, loRec, hiRec);
    int fLen = loRec.size();

    QVector<double> current = m_approxCoeffs;

    for (int lev = m_levels - 1; lev >= 0; --lev) {
        if (lev >= m_detailCoeffs.size()) continue;
        int half = current.size();
        int n = half * 2;
        QVector<double> reconstructed(n, 0.0);

        for (int i = 0; i < half; ++i) {
            for (int k = 0; k < fLen; ++k) {
                int idx = (2 * i + k) % n;
                reconstructed[idx] += loRec[k] * current[i] + hiRec[k] * m_detailCoeffs[lev][i];
            }
        }
        current = reconstructed;
    }
    return current;
}

/* ---- Estimate sigma via MAD ---- */

double WaveletDenoiser16::estimateSigma(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;
    QVector<double> absCoeffs(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) absCoeffs[i] = qAbs(coeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];
    return median / 0.6745;  // MAD scaling for Gaussian
}

/* ---- Bayesian shrinkage with inter-scale dependency ---- */

void WaveletDenoiser16::bayesianShrinkage()
{
    if (m_noiseStd <= 0.0 && !m_detailCoeffs.isEmpty())
        m_noiseStd = estimateSigma(m_detailCoeffs[0]);

    double sigma2 = m_noiseStd * m_noiseStd;
    if (sigma2 < 1e-20) return;

    for (int lev = 0; lev < m_levels; ++lev) {
        int n = m_detailCoeffs[lev].size();
        if (n == 0) continue;

        // Estimate signal variance per level using local neighborhood
        double levelPower = 0.0;
        for (int i = 0; i < n; ++i) levelPower += m_detailCoeffs[lev][i] * m_detailCoeffs[lev][i];
        levelPower /= n;

        // Inter-scale dependency: weight from parent coefficient
        QVector<double> parentWeight(n, 1.0);
        if (lev > 0 && !m_detailCoeffs[lev - 1].isEmpty()) {
            int parentSize = m_detailCoeffs[lev - 1].size();
            for (int i = 0; i < n; ++i) {
                int pi = i / 2;
                if (pi < parentSize) {
                    double parentAbs = qAbs(m_detailCoeffs[lev - 1][pi]);
                    parentWeight[i] = 1.0 + parentAbs / (m_noiseStd + 1e-10);
                }
            }
        }

        // Bayesian shrinkage: posterior mean under Gaussian prior
        for (int i = 0; i < n; ++i) {
            double c = m_detailCoeffs[lev][i];
            double localVar = levelPower - sigma2;
            if (localVar < 1e-20) localVar = 1e-20;

            // Inter-scale weighted shrinkage factor
            double shrink = localVar / (localVar + sigma2 * parentWeight[i]);
            shrink = qBound(0.0, shrink, 1.0);

            m_detailCoeffs[lev][i] = c * shrink;
        }
    }
}

/* ---- Signal power helper ---- */

double WaveletDenoiser16::signalPower(const QVector<double>& s) const
{
    double p = 0.0;
    for (double v : s) p += v * v;
    return p / qMax(s.size(), 1);
}

/* ---- Estimate noise from signal ---- */

void WaveletDenoiser16::estimateNoise(const QVector<double>& signal)
{
    forwardDWT(signal);
    if (!m_detailCoeffs.isEmpty())
        m_noiseStd = estimateSigma(m_detailCoeffs[0]);
}

/* ---- Main denoising ---- */

QVector<double> WaveletDenoiser16::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty()) return {};

    // Forward DWT
    forwardDWT(signal);

    // Estimate noise if not set
    if (m_noiseStd <= 0.0 && !m_detailCoeffs.isEmpty())
        m_noiseStd = estimateSigma(m_detailCoeffs[0]);

    double inPower = signalPower(signal);
    double inputSNR = 10.0 * qLog10(qMax(inPower / (m_noiseStd * m_noiseStd), 1e-10));

    // Apply Bayesian shrinkage with inter-scale dependency
    bayesianShrinkage();

    // Inverse DWT
    m_reconstructed = inverseDWT();

    // Compute output SNR
    QVector<double> noise(signal.size(), 0.0);
    for (int i = 0; i < qMin(signal.size(), m_reconstructed.size()); ++i)
        noise[i] = signal[i] - m_reconstructed[i];
    double noisePower = signalPower(noise);
    double outPower = signalPower(m_reconstructed);
    double outputSNR = 10.0 * qLog10(qMax(outPower / qMax(noisePower, 1e-20), 1e-10));

    double elapsed = timer.elapsed();
    m_stats.signalLength = signal.size();
    m_stats.numLevels = m_levels;
    m_stats.inputSNR = inputSNR;
    m_stats.outputSNR = outputSNR;
    m_stats.noiseStd = m_noiseStd;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoisingDone(signal.size(), inputSNR, outputSNR, elapsed);

    return m_reconstructed;
}

/* ---- Accessors ---- */

QVector<double> WaveletDenoiser16::coefficients(int level) const
{
    if (level < 0 || level >= m_detailCoeffs.size()) return {};
    return m_detailCoeffs[level];
}

QVector<double> WaveletDenoiser16::reconstructed() const { return m_reconstructed; }

/* ---- Reset ---- */

void WaveletDenoiser16::resetStatistics()
{
    m_detailCoeffs.clear();
    m_approxCoeffs.clear();
    m_reconstructed.clear();
    m_noiseStd = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
