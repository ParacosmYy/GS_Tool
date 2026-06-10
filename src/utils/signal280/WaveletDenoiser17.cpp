/**
 * @file WaveletDenoiser17.cpp
 * @brief WaveletDenoiser17 实现
 *
 * 实现小波去噪器：多小波基与多分辨率阈值自适应的复合信号分析去噪。
 */

#include "utils/signal280/WaveletDenoiser17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser17::WaveletDenoiser17(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser17::~WaveletDenoiser17() = default;

/* ---- Configuration ---- */

void WaveletDenoiser17::setBasis(Basis b) { m_basis = b; }
void WaveletDenoiser17::setLevels(int levels) { m_levels = qBound(1, levels, 15); }
void WaveletDenoiser17::setThresholdMethod(ThresholdMethod method) { m_threshMethod = method; }
void WaveletDenoiser17::setThreshold(double t) { m_manualThreshold = qBound(0.0, t, 1e10); }

/* ---- Get low-pass filter coefficients ---- */

void WaveletDenoiser17::getLowPassFilter(QVector<double>& h) const
{
    switch (m_basis) {
    case Basis::Haar:
        h = {0.7071067811865476, 0.7071067811865476};
        break;
    case Basis::Daubechies4:
        h = {0.4829629131445341, 0.8365163037378079, 0.2241438680420134, -0.1294095225512603};
        break;
    case Basis::Daubechies8:
        h = {0.2303778133088964, 0.7148465705529154, 0.6308807679398587, -0.0279837694168599,
             -0.1870348117190931, 0.0308413818355607, 0.0328830116668852, -0.0105974017850690};
        break;
    case Basis::Symlet6:
        h = {0.0154041093270274, 0.0034907120842174, -0.1179901111481909, -0.0483117425856330,
             0.4910559419267466, 0.7876411410301940, 0.3379294217276218, -0.0726375227864625,
             -0.0210602925123001, 0.0447249017706658, 0.0017677118642428, -0.0078007083250341};
        break;
    case Basis::Coiflet3:
        h = {-0.0026818145689583, -0.0010473848886829, 0.0126360043828490, 0.0305151947118837,
             -0.0678926935013727, -0.0495528349371274, 0.1366464441060843, 0.5808155607245420,
             0.4698200192725668, 0.0290194030403260};
        break;
    }
}

/* ---- Get high-pass filter (alternating flip) ---- */

void WaveletDenoiser17::getHighPassFilter(QVector<double>& g) const
{
    QVector<double> h;
    getLowPassFilter(h);
    int n = h.size();
    g.resize(n);
    for (int i = 0; i < n; ++i)
        g[i] = (i % 2 == 0) ? h[n - 1 - i] : -h[n - 1 - i];
}

/* ---- Single-level decomposition ---- */

void WaveletDenoiser17::decompose(const QVector<double>& input,
                                    QVector<double>& approx,
                                    QVector<double>& detail) const
{
    QVector<double> h, g;
    getLowPassFilter(h);
    getHighPassFilter(g);

    int n = input.size();
    int fLen = h.size();
    int half = n / 2;

    approx.resize(half, 0.0);
    detail.resize(half, 0.0);

    for (int i = 0; i < half; ++i) {
        double aSum = 0.0, dSum = 0.0;
        for (int k = 0; k < fLen; ++k) {
            int idx = (2 * i + k) % n;
            aSum += h[k] * input[idx];
            dSum += g[k] * input[idx];
        }
        approx[i] = aSum;
        detail[i] = dSum;
    }
}

/* ---- Single-level reconstruction ---- */

void WaveletDenoiser17::reconstruct(const QVector<double>& approx,
                                      const QVector<double>& detail,
                                      QVector<double>& output) const
{
    QVector<double> h, g;
    getLowPassFilter(h);
    getHighPassFilter(g);

    int fLen = h.size();
    int half = approx.size();
    int n = half * 2;
    output.resize(n, 0.0);

    for (int i = 0; i < half; ++i) {
        for (int k = 0; k < fLen; ++k) {
            int idx = (2 * i + k) % n;
            output[idx] += h[k] * approx[i] + g[k] * detail[i];
        }
    }
}

/* ---- Pad signal to next power of 2 ---- */

QVector<double> WaveletDenoiser17::padSignal(const QVector<double>& input, int& paddedLen) const
{
    int n = input.size();
    paddedLen = 1;
    while (paddedLen < n) paddedLen *= 2;

    QVector<double> padded(paddedLen, 0.0);
    for (int i = 0; i < n; ++i) padded[i] = input[i];
    // Mirror padding for the tail
    for (int i = n; i < paddedLen; ++i)
        padded[i] = input[2 * n - i - 1];
    return padded;
}

/* ---- Estimate noise sigma (MAD of finest coefficients) ---- */

double WaveletDenoiser17::estimateNoiseSigma(const QVector<double>& input) const
{
    if (input.size() < 4) return 0.0;
    // Compute first differences as noise proxy
    QVector<double> diff;
    for (int i = 1; i < input.size(); ++i)
        diff.append(input[i] - input[i - 1]);

    QVector<double> sorted = diff;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[sorted.size() / 2];
    // MAD estimator: sigma = median(|d|) / 0.6745
    double mad = 0.0;
    for (auto& v : sorted) v = qAbs(v - median);
    std::sort(sorted.begin(), sorted.end());
    mad = sorted[sorted.size() / 2];
    return mad / 0.6745;
}

/* ---- Universal threshold (VisuShrink) ---- */

double WaveletDenoiser17::universalThreshold(const QVector<double>& coefficients) const
{
    int n = coefficients.size();
    if (n == 0) return 0.0;
    // sigma * sqrt(2 * ln(n))
    double sigma = 0.0;
    for (auto c : coefficients) sigma += c * c;
    sigma = qSqrt(sigma / n);
    return sigma * qSqrt(2.0 * qLn(static_cast<double>(n)));
}

/* ---- BayesShrink threshold ---- */

double WaveletDenoiser17::bayesThreshold(const QVector<double>& coefficients,
                                            double noiseSigma) const
{
    if (coefficients.isEmpty() || noiseSigma < 1e-15) return 0.0;
    double sigY = 0.0;
    for (auto c : coefficients) sigY += c * c;
    sigY = qSqrt(sigY / coefficients.size());
    double sigX = qSqrt(qMax(0.0, sigY * sigY - noiseSigma * noiseSigma));
    return (sigX < 1e-15) ? qSqrt(2.0 * qLn(static_cast<double>(coefficients.size()))) * noiseSigma
                           : noiseSigma * noiseSigma / sigX;
}

/* ---- Apply thresholding ---- */

double WaveletDenoiser17::applyThreshold(double value, double threshold) const
{
    double absV = qAbs(value);
    switch (m_threshMethod) {
    case ThresholdMethod::Soft:
        return (absV > threshold) ? (value > 0 ? absV - threshold : -(absV - threshold)) : 0.0;
    case ThresholdMethod::Hard:
        return (absV > threshold) ? value : 0.0;
    case ThresholdMethod::SemiSoft:
        return (absV > 2.0 * threshold) ? value
               : (absV > threshold) ? value * (absV - threshold) / threshold
               : 0.0;
    case ThresholdMethod::Garrote:
        return (absV > threshold) ? value - threshold * threshold / value : 0.0;
    }
    return 0.0;
}

/* ---- Main denoise ---- */

WaveletDenoiser17::DenoiseResult WaveletDenoiser17::denoise(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DenoiseResult result;
    int n = input.size();
    if (n < 4) {
        result.denoised = input;
        return result;
    }

    // Pad to power of 2
    int paddedLen = 0;
    auto padded = padSignal(input, paddedLen);

    // Estimate noise
    double noiseSigma = estimateNoiseSigma(input);

    // Multi-level decomposition
    int levels = qMin(m_levels, static_cast<int>(qLn(paddedLen) / qLn(2)) - 1);
    result.numLevels = levels;

    QVector<QVector<double>> details(levels);
    QVector<double> current = padded;

    for (int lev = 0; lev < levels; ++lev) {
        QVector<double> approx, detail;
        decompose(current, approx, detail);
        details[lev] = detail;
        current = approx;
    }

    // Multi-resolution threshold adaptation
    result.totalCoefficients = 0;
    result.coefficientsZeroed = 0;
    double globalThreshold = m_manualThreshold;

    for (int lev = 0; lev < levels; ++lev) {
        double thresh = (globalThreshold > 0.0)
                            ? globalThreshold
                            : bayesThreshold(details[lev], noiseSigma);

        // Scale threshold by level (finer levels get higher threshold)
        double levelScale = qSqrt(static_cast<double>(1 << (lev + 1)));
        thresh *= levelScale / qSqrt(static_cast<double>(1 << levels));

        result.totalCoefficients += details[lev].size();
        for (int i = 0; i < details[lev].size(); ++i) {
            if (qAbs(details[lev][i]) <= thresh) result.coefficientsZeroed++;
            details[lev][i] = applyThreshold(details[lev][i], thresh);
        }

        if (lev == 0) result.threshold = thresh;
    }

    // Reconstruction
    QVector<double> reconApprox = current;
    for (int lev = levels - 1; lev >= 0; --lev) {
        QVector<double> recon;
        reconstruct(reconApprox, details[lev], recon);
        reconApprox = recon;
    }

    // Trim to original length
    result.denoised.resize(n);
    for (int i = 0; i < n; ++i)
        result.denoised[i] = reconApprox[i];

    // Noise estimate
    result.noiseEstimate.resize(n);
    for (int i = 0; i < n; ++i)
        result.noiseEstimate[i] = input[i] - result.denoised[i];

    // SNR estimates
    double sigPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < n; ++i) {
        sigPow += result.denoised[i] * result.denoised[i];
        noisePow += result.noiseEstimate[i] * result.noiseEstimate[i];
    }
    result.inputSNR = 10.0 * qLn(sigPow / (noisePow + 1e-30) + 1e-30) / M_LN10;
    result.outputSNR = result.inputSNR;

    double elapsed = timer.elapsed();
    m_stats.signalLength = n;
    m_stats.numLevels = levels;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoiseDone(n, levels, result.threshold, elapsed);

    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
