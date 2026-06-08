/**
 * @file FilterDesign5.cpp
 * @brief FilterDesign5 实现
 *
 * 实现IIR滤波器设计：双线性变换、Butterworth/Chebyshev原型、频率预畸变。
 */

#include "utils/signal223/FilterDesign5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FilterDesign5::FilterDesign5(QObject *parent) : QObject(parent) {}
FilterDesign5::~FilterDesign5() = default;

/* ---- Configuration ---- */

void FilterDesign5::setSampleRate(int sampleRate)
{
    m_sampleRate = qMax(1000, sampleRate);
    m_stats.sampleRate = m_sampleRate;
}

/* ---- Pre-warp frequency ---- */

double FilterDesign5::prewarpFrequency(double freq) const
{
    // wa = 2*Fs * tan(pi*f/Fs)
    return 2.0 * m_sampleRate * qTan(M_PI * freq / m_sampleRate);
}

/* ---- Butterworth analog prototype poles ---- */

QVector<QVector<double>> FilterDesign5::butterworthPoles(int n) const
{
    QVector<QVector<double>> poles; // [re, im] pairs
    for (int k = 0; k < n; ++k) {
        double angle = M_PI * (2 * k + n + 1) / (2 * n);
        poles.append({qCos(angle), qSin(angle)});
    }
    return poles;
}

/* ---- Chebyshev Type I analog prototype poles ---- */

QVector<QVector<double>> FilterDesign5::chebyshevPoles(int n,
                                                          double ripple) const
{
    double eps = qSqrt(qPow(10.0, ripple / 10.0) - 1.0);
    double gamma = qPow(eps + qSqrt(1.0 + eps * eps), 1.0 / n);
    double sigma = 0.5 * (gamma - 1.0 / gamma);
    double omega = 0.5 * (gamma + 1.0 / gamma);

    QVector<QVector<double>> poles;
    for (int k = 0; k < n; ++k) {
        double angle = M_PI * (2 * k + 1) / (2 * n);
        poles.append({sigma * qCos(angle), omega * qSin(angle)});
    }
    return poles;
}

/* ---- Expand poles into polynomial ---- */

QVector<double> FilterDesign5::expandPoles(
    const QVector<QVector<double>>& poles) const
{
    // Start with polynomial [1]
    QVector<double> poly = {1.0};

    for (auto& p : poles) {
        double re = p[0], im = p[1];
        if (qAbs(im) < 1e-12) {
            // Real pole: multiply by (s - re)
            QVector<double> factor = {-re, 1.0};
            poly = polyConvolve(poly, factor);
        } else if (im > 0) {
            // Complex conjugate pair: (s - re - j*im)(s - re + j*im)
            // = s^2 - 2*re*s + (re^2 + im^2)
            double rr = re * re + im * im;
            QVector<double> factor = {rr, -2.0 * re, 1.0};
            poly = polyConvolve(poly, factor);
        }
    }
    return poly;
}

/* ---- Polynomial convolution ---- */

QVector<double> FilterDesign5::polyConvolve(const QVector<double>& p1,
                                               const QVector<double>& p2) const
{
    int n1 = p1.size(), n2 = p2.size();
    QVector<double> result(n1 + n2 - 1, 0.0);
    for (int i = 0; i < n1; ++i)
        for (int j = 0; j < n2; ++j)
            result[i + j] += p1[i] * p2[j];
    return result;
}

/* ---- Bilinear transform ---- */

void FilterDesign5::bilinearTransform(QVector<double>& b, QVector<double>& a,
                                        const QVector<QVector<double>>& poles,
                                        double warpedFreq) const
{
    // Scale poles by warped cutoff frequency
    QVector<QVector<double>> scaledPoles;
    for (auto& p : poles)
        scaledPoles.append({p[0] * warpedFreq, p[1] * warpedFreq});

    // Analog denominator polynomial (from poles)
    QVector<double> aAnalogue = expandPoles(scaledPoles);
    int n = aAnalogue.size() - 1;

    // Numerator for LP: s^n * gain
    // Gain: product of |poles| to normalize DC
    double gain = 1.0;
    for (auto& p : scaledPoles)
        gain *= qSqrt(p[0] * p[0] + p[1] * p[1]);
    QVector<double> bAnalogue(n + 1, 0.0);
    bAnalogue[0] = gain;

    // Bilinear: s = 2*Fs * (z-1)/(z+1)
    // Substitute and expand
    double fs2 = 2.0 * m_sampleRate;

    // Build numerator and denominator in z-domain
    a.resize(n + 1, 0.0);
    b.resize(n + 1, 0.0);

    // For each coefficient aAnalogue[k] * s^(n-k):
    // s^m maps to sum of (z-1)^m_coeff * binomial / (z+1)^n
    QVector<double> numZeros(n + 1, 0.0);  // (z+1)^n coefficients
    QVector<double> numOnes(n + 1, 0.0);   // (z-1)^k*(z+1)^(n-k) scaled

    // Simple approach: use substitution matrix
    for (int k = 0; k <= n; ++k) {
        // s^k contribution via bilinear: (2Fs)^k * (z-1)^k / (z+1)^k
        // Combined denominator: (z+1)^n
        for (int i = 0; i <= k; ++i) {
            double binom = 1.0;
            for (int j = 0; j < i; ++j)
                binom = binom * (k - j) / (j + 1);
            double sign = ((k - i) % 2 == 0) ? 1.0 : -1.0;

            // Remaining (z+1)^(n-k) factor
            for (int j = 0; j <= n - k; ++j) {
                double binom2 = 1.0;
                for (int l = 0; l < j; ++l)
                    binom2 = binom2 * (n - k - l) / (l + 1);
                int idx = i + j;
                double val = aAnalogue[n - k] * qPow(fs2, k) * binom * sign * binom2;
                a[idx] += val;
            }
        }
    }

    // Numerator: gain * (z+1)^n
    double binomN = 1.0;
    for (int i = 0; i <= n; ++i) {
        if (i > 0) binomN = binomN * (n - i + 1) / i;
        b[i] = gain * binomN;
    }

    // Normalize by a[0]
    double norm = (qAbs(a[0]) > 1e-30) ? a[0] : 1.0;
    for (int i = 0; i <= n; ++i) { b[i] /= norm; a[i] /= norm; }
}

/* ---- Design ---- */

FilterDesign5::FilterCoeffs FilterDesign5::design(FilterType type,
    Prototype proto, int order, double cutoffFreq, double rippleDb) const
{
    QElapsedTimer timer;
    timer.start();

    FilterCoeffs coeffs;
    coeffs.order = order;
    coeffs.type = type;
    coeffs.cutoffFreq = cutoffFreq;

    // Get analog prototype poles
    QVector<QVector<double>> poles;
    if (proto == ChebyshevType1)
        poles = chebyshevPoles(order, rippleDb);
    else
        poles = butterworthPoles(order);

    // Pre-warp cutoff frequency
    double warpedFreq = prewarpFrequency(cutoffFreq);

    // Apply bilinear transform
    QVector<double> b, a;
    bilinearTransform(b, a, poles, warpedFreq);

    // For highpass: apply frequency transformation z -> -z
    if (type == HighPass) {
        for (int i = 0; i < a.size(); ++i)
            if (i % 2 != 0) a[i] = -a[i];
        for (int i = 0; i < b.size(); ++i)
            if (i % 2 != 0) b[i] = -b[i];
    }

    coeffs.b = b;
    coeffs.a = a;

    const_cast<FilterDesign5*>(this)->m_stats.lastOrder = order;
    const_cast<FilterDesign5*>(this)->m_stats.totalOps++;
    const_cast<FilterDesign5*>(this)->m_timeSum += timer.elapsed();
    const_cast<FilterDesign5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<FilterDesign5*>(this)->filterDesigned(order, cutoffFreq,
                                                          timer.elapsed());
    return coeffs;
}

/* ---- Design band ---- */

FilterDesign5::FilterCoeffs FilterDesign5::designBand(FilterType type,
    Prototype proto, int order, double lowFreq, double highFreq,
    double rippleDb) const
{
    // Design LPF at highFreq, then apply band transformation
    double centerFreq = qSqrt(lowFreq * highFreq);
    FilterCoeffs coeffs = design(LowPass, proto, order, highFreq, rippleDb);

    if (type == BandPass || type == BandStop) {
        // Notch the lowpass to create bandpass
        // Simplified: cascade LPF(high) with HPF(low)
        FilterCoeffs hp = design(HighPass, proto, order, lowFreq, rippleDb);
        int n = coeffs.b.size();
        // Convolve coefficients for cascade
        coeffs.b = polyConvolve(coeffs.b, hp.b);
        coeffs.a = polyConvolve(coeffs.a, hp.a);
        coeffs.cutoffFreq = centerFreq;
    }
    return coeffs;
}

/* ---- Apply filter ---- */

QVector<double> FilterDesign5::apply(const QVector<double>& input,
                                       const FilterCoeffs& coeffs) const
{
    int n = input.size();
    int nb = coeffs.b.size();
    int na = coeffs.a.size();
    QVector<double> output(n, 0.0);

    // Direct Form II transposed
    for (int i = 0; i < n; ++i) {
        double y = coeffs.b[0] * input[i];
        for (int j = 1; j < nb; ++j)
            if (i >= j) y += coeffs.b[j] * input[i - j];
        for (int j = 1; j < na; ++j)
            if (i >= j) y -= coeffs.a[j] * output[i - j];
        output[i] = y;
    }
    return output;
}

/* ---- Frequency response ---- */

QVector<FilterDesign5::FreqResponse> FilterDesign5::frequencyResponse(
    const FilterCoeffs& coeffs, int numPoints) const
{
    QVector<FreqResponse> response(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double freq = (m_sampleRate / 2.0) * i / numPoints;
        double w = 2.0 * M_PI * freq / m_sampleRate;

        // Evaluate H(z) at z = e^(jw)
        double reZ = qCos(w), imZ = qSin(w);
        double numRe = 0.0, numIm = 0.0;
        double denRe = 0.0, denIm = 0.0;

        for (int k = 0; k < coeffs.b.size(); ++k) {
            double mag = qPow(reZ * reZ + imZ * imZ, k / 2.0);
            double ang = k * w;
            numRe += coeffs.b[k] * qCos(ang);
            numIm += coeffs.b[k] * qSin(ang);
        }
        for (int k = 0; k < coeffs.a.size(); ++k) {
            double ang = k * w;
            denRe += coeffs.a[k] * qCos(ang);
            denIm += coeffs.a[k] * qSin(ang);
        }

        double denMag = qSqrt(denRe * denRe + denIm * denIm);
        double numMag = qSqrt(numRe * numRe + numIm * numIm);
        double mag = (denMag > 1e-30) ? numMag / denMag : 0.0;

        response[i].frequency = freq;
        response[i].magnitude = 20.0 * qLn(qMax(1e-30, mag)) / qLn(10.0);
        response[i].phase = qAtan2(numIm * denRe - numRe * denIm,
                                    numRe * denRe + numIm * denIm);
    }
    return response;
}

/* ---- Reset ---- */

void FilterDesign5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
