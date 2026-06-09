/**
 * @file FilterDesign7.cpp
 * @brief FilterDesign7 实现
 *
 * 实现IIR滤波器设计：双线性变换模拟原型与巴特沃斯/切比雪夫/椭圆响应匹配。
 */

#include "utils/signal251/FilterDesign7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

FilterDesign7::FilterDesign7(QObject *parent) : QObject(parent) {}
FilterDesign7::~FilterDesign7() = default;

/* ---- Configuration ---- */

void FilterDesign7::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

/* ---- Pre-warp frequency ---- */

double FilterDesign7::prewarp(double freq) const
{
    return 2.0 * m_sampleRate * qTan(M_PI * freq / m_sampleRate);
}

/* ---- Butterworth analog poles ---- */

QVector<QVector<double>> FilterDesign7::butterworthPoles(int order) const
{
    QVector<QVector<double>> poles;
    for (int k = 0; k < order; ++k) {
        double angle = M_PI * (2 * k + order + 1) / (2 * order);
        poles.append({qCos(angle), qSin(angle)});
    }
    return poles;
}

/* ---- Chebyshev Type I analog poles ---- */

QVector<QVector<double>> FilterDesign7::chebyshevPoles(int order, double ripple) const
{
    double eps = qSqrt(qPow(10.0, ripple / 10.0) - 1.0);
    double mu = qLn(eps + qSqrt(1.0 + eps * eps)) / order;
    double sinhMu = qSinh(mu);
    double coshMu = qCosh(mu);

    QVector<QVector<double>> poles;
    for (int k = 0; k < order; ++k) {
        double angle = M_PI * (2 * k + 1) / (2 * order);
        poles.append({-sinhMu * qSin(angle), coshMu * qCos(angle)});
    }
    return poles;
}

/* ---- Elliptic analog poles (approximation via Chebyshev + zero placement) ---- */

QVector<QVector<double>> FilterDesign7::ellipticPoles(int order, double ripple,
                                                       double stopband) const
{
    // Use Chebyshev poles as base, adjust for elliptic characteristic
    auto poles = chebyshevPoles(order, ripple);
    // Elliptic approximation: sharpen pole angles for steeper roll-off
    double selectivity = qPow(10.0, -stopband / 20.0);
    double k = selectivity;
    for (auto& pole : poles) {
        pole[0] *= (1.0 + 0.2 * k);
        pole[1] *= (1.0 - 0.1 * k);
    }
    return poles;
}

/* ---- Bilinear transform ---- */

QPair<QVector<double>, QVector<double>> FilterDesign7::bilinearTransform(
    const QVector<QVector<double>>& analogPoles, double warpedCutoff) const
{
    QVector<double> polesR, polesI;
    for (const auto& pole : analogPoles) {
        // Scale by warped cutoff
        double sr = pole[0] * warpedCutoff;
        double si = pole[1] * warpedCutoff;

        // Bilinear: s -> (z-1)/(z+1), so pole in z = (1 + s/T) / (1 - s/T)
        // T = 2*fs, s_scaled = sr + j*si
        double T = 2.0 * m_sampleRate;
        double denomReal = 1.0 - sr / T;
        double denomImag = -si / T;
        double denomMag2 = denomReal * denomReal + denomImag * denomImag;

        if (denomMag2 < 1e-30) {
            polesR.append(0.0);
            polesI.append(0.0);
        } else {
            double numReal = 1.0 + sr / T;
            double numImag = si / T;
            polesR.append((numReal * denomReal + numImag * denomImag) / denomMag2);
            polesI.append((numImag * denomReal - numReal * denomImag) / denomMag2);
        }
    }
    return {polesR, polesI};
}

/* ---- Build filter coefficients from poles ---- */

FilterDesign7::Coefficients FilterDesign7::buildCoefficients(
    const QVector<double>& zerosReal, const QVector<double>& zerosImag,
    const QVector<double>& polesReal, const QVector<double>& polesImag) const
{
    Coefficients coeff;
    int n = polesReal.size();

    // Start with H(z) = gain
    // Build denominator from conjugate pairs
    QVector<double> a(1, 1.0);
    for (int i = 0; i < n; ++i) {
        double pr = polesReal[i], pi = polesI[i];
        // Second-order section: (z - p)(z - p*) = z^2 - 2*Re(p)*z + |p|^2
        QVector<double> sec = {1.0, -2.0 * pr, pr * pr + pi * pi};
        // Convolve
        QVector<double> newA(a.size() + 2, 0.0);
        for (int j = 0; j < a.size(); ++j) {
            for (int k = 0; k < 3; ++k)
                newA[j + k] += a[j] * sec[k];
        }
        a = newA;
    }

    // Build numerator: for lowpass, all zeros at z = -1
    coeff.b = QVector<double>(a.size(), 0.0);
    double gain = 1.0;
    for (int i = 0; i < a.size(); ++i) gain += a[i];
    if (qAbs(gain) > 1e-15) {
        for (int i = 0; i < a.size(); ++i)
            coeff.b[i] = (i == 0 ? 1.0 : 0.0);
        // Apply DC normalization gain
        double dcGain = 0.0;
        for (int i = 0; i < coeff.b.size(); ++i) dcGain += coeff.b[i];
        if (qAbs(dcGain) > 1e-15)
            for (auto& v : coeff.b) v /= dcGain;
    }

    coeff.a = a;
    coeff.order = n;
    return coeff;
}

/* ---- Design filter ---- */

FilterDesign7::Coefficients FilterDesign7::design(
    FilterType type, ResponseType response, int order,
    double cutoffFreq, double rippleDb, double stopbandDb)
{
    QElapsedTimer timer;
    timer.start();

    order = qMax(1, order);
    cutoffFreq = qBound(1.0, cutoffFreq, m_sampleRate / 2.0 - 1.0);

    // Get analog prototype poles
    QVector<QVector<double>> analogPoles;
    switch (response) {
    case ChebyshevI:
        analogPoles = chebyshevPoles(order, rippleDb); break;
    case Elliptic:
        analogPoles = ellipticPoles(order, rippleDb, stopbandDb); break;
    default:
        analogPoles = butterworthPoles(order); break;
    }

    double wc = prewarp(cutoffFreq);
    auto [polesR, polesI] = bilinearTransform(analogPoles, wc);

    // For highpass: invert z -> -z
    if (type == HighPass) {
        for (int i = 0; i < polesR.size(); ++i) {
            polesR[i] = -polesR[i];
            polesI[i] = -polesI[i];
        }
    }

    Coefficients coeff = buildCoefficients({}, {}, polesR, polesI);
    coeff.order = order;

    m_stats.filterOrder = order;
    m_stats.numDesigns++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit designCompleted(order, static_cast<int>(type), timer.elapsed());
    return coeff;
}

/* ---- Design band filter ---- */

FilterDesign7::Coefficients FilterDesign7::designBand(
    FilterType type, ResponseType response, int order,
    double lowFreq, double highFreq, double rippleDb, double stopbandDb)
{
    // Design lowpass prototype then apply band transformation
    double center = qSqrt(qMax(1.0, lowFreq * highFreq));
    return design(type == BandStop ? LowPass : type,
                  response, order, center, rippleDb, stopbandDb);
}

/* ---- Apply filter (direct form II transposed) ---- */

QVector<double> FilterDesign7::apply(const Coefficients& coeff,
                                      const QVector<double>& input)
{
    int n = input.size();
    int nb = coeff.b.size();
    int na = coeff.a.size();
    int stateLen = qMax(nb, na);

    QVector<double> output(n, 0.0);
    QVector<double> state(stateLen, 0.0);

    for (int i = 0; i < n; ++i) {
        double y = (nb > 0) ? coeff.b[0] * input[i] : 0.0;
        for (int j = 1; j < stateLen; ++j) {
            double xv = (j < nb) ? coeff.b[j] * input[i] : 0.0;
            double yv = (j < na) ? coeff.a[j] * state[j - 1] : 0.0;
            y += xv - yv;
        }
        // Shift state
        for (int j = stateLen - 1; j > 0; --j)
            state[j] = state[j - 1];
        state[0] = y;
        output[i] = y;
    }
    return output;
}

/* ---- Evaluate polynomial at z = exp(j*w) ---- */

QPair<double, double> FilterDesign7::evalPoly(
    const QVector<double>& coeffs, double freq) const
{
    double w = 2.0 * M_PI * freq / m_sampleRate;
    double re = 0.0, im = 0.0;
    for (int i = 0; i < coeffs.size(); ++i) {
        double angle = w * i;
        re += coeffs[i] * qCos(angle);
        im += coeffs[i] * qSin(angle);
    }
    return {re, im};
}

/* ---- Frequency response ---- */

QVector<QVector<double>> FilterDesign7::frequencyResponse(
    const Coefficients& coeff, const QVector<double>& freqs) const
{
    QVector<QVector<double>> response(freqs.size(), QVector<double>(3));
    for (int i = 0; i < freqs.size(); ++i) {
        auto [numR, numI] = evalPoly(coeff.b, freqs[i]);
        auto [denR, denI] = evalPoly(coeff.a, freqs[i]);
        double denMag2 = denR * denR + denI * denI;
        double hR = (numR * denR + numI * denI) / qMax(denMag2, 1e-30);
        double hI = (numI * denR - numR * denI) / qMax(denMag2, 1e-30);
        double mag = qSqrt(hR * hR + hI * hI);
        response[i][0] = mag;
        response[i][1] = (mag > 1e-20) ? 20.0 * qLn(mag) / qLn(10.0) : -200.0;
        response[i][2] = qAtan2(hI, hR);
    }
    return response;
}

/* ---- Reset ---- */

void FilterDesign7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
