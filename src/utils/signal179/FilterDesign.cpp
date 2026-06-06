/**
 * @file FilterDesign.cpp
 * @brief FilterDesign 实现
 *
 * 实现IIR/FIR滤波器设计：Butterworth/Chebyshev/Elliptic、双线性变换、频率响应。
 */

#include "utils/signal179/FilterDesign.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

FilterDesign::FilterDesign(QObject *parent)
    : QObject(parent)
{
}

FilterDesign::~FilterDesign() = default;

/* ---- Configuration ---- */

void FilterDesign::setOrder(int order) { m_order = qMax(1, order); }
void FilterDesign::setSampleRate(double fs) { m_fs = qMax(1.0, fs); }
void FilterDesign::setCutoffFreq(double fc) { m_fc = qBound(1.0, fc, m_fs / 2.0 - 1.0); }
void FilterDesign::setBandwidth(double lowFc, double highFc) {
    m_fcLow = qBound(1.0, lowFc, m_fs / 2.0);
    m_fcHigh = qBound(m_fcLow + 1.0, highFc, m_fs / 2.0 - 1.0);
}
void FilterDesign::setPassbandRipple(double dB) { m_passbandRipple = qMax(0.1, dB); }
void FilterDesign::setStopbandAttenuation(double dB) { m_stopbandAtten = qMax(1.0, dB); }

/* ---- Prewarp (bilinear transform) ---- */

double FilterDesign::prewarp(double analogFreq) const
{
    /* ω_a = 2*fs * tan(π*ω_d / fs) */
    return 2.0 * m_fs * qTan(M_PI * analogFreq / m_fs);
}

/* ---- Butterworth analog poles ---- */

QVector<QPair<double, double>> FilterDesign::butterworthPoles(int n) const
{
    QVector<QPair<double, double>> poles;
    for (int k = 0; k < n; ++k) {
        double theta = M_PI * (2 * k + 1) / (2.0 * n) + M_PI / 2.0;
        poles.append({qCos(theta), qSin(theta)});
    }
    return poles;
}

/* ---- Bilinear transform: s-plane -> z-plane ---- */

QVector<QPair<double, double>> FilterDesign::bilinearTransform(
    const QVector<QPair<double, double>>& poles) const
{
    /* Pre-warp cutoff */
    double wc = prewarp(m_fc);
    double T = 1.0 / m_fs;
    double c = 2.0 / T; /* = 2*fs */

    QVector<QPair<double, double>> zPoles;
    for (const auto& p : poles) {
        double re = p.first * wc;
        double im = p.second * wc;
        /* z = (c + s) / (c - s) where s = re + j*im */
        double denomRe = c - re;
        double denomIm = -im;
        double numRe = c + re;
        double numIm = im;

        double dMag2 = denomRe * denomRe + denomIm * denomIm;
        double zRe = (numRe * denomRe + numIm * denomIm) / dMag2;
        double zIm = (numIm * denomRe - numRe * denomIm) / dMag2;
        zPoles.append({zRe, zIm});
    }
    return zPoles;
}

/* ---- Main design ---- */

FilterDesign::Coefficients FilterDesign::design(Type type, Approximation approx)
{
    QElapsedTimer timer;
    timer.start();

    Coefficients coeff;
    coeff.order = m_order;
    coeff.type = type;

    int n = m_order;

    /* Get analog poles based on approximation */
    QVector<QPair<double, double>> poles;
    double eps = qSqrt(qPow(10.0, m_passbandRipple / 10.0) - 1.0);

    switch (approx) {
    case Butterworth:
        poles = butterworthPoles(n);
        break;
    case ChebyshevI:
        poles = butterworthPoles(n);
        /* Chebyshev I: poles on ellipse, scale by eps */
        for (auto& p : poles) {
            double xi = qAsinh(1.0 / eps) / n;
            p.first *= qSinh(xi);
            p.second *= qCosh(xi);
        }
        break;
    case ChebyshevII:
        poles = butterworthPoles(n);
        /* Inverse Chebyshev: use stopband attenuation */
        {
            double eps_s = 1.0 / qSqrt(qPow(10.0, m_stopbandAtten / 10.0) - 1.0);
            for (auto& p : poles) {
                double xi = qAsinh(eps_s) / n;
                p.first *= qSinh(xi);
                p.second *= qCosh(xi);
            }
        }
        break;
    case Elliptic:
        poles = butterworthPoles(n);
        /* Elliptic: combined Chebyshev + inverse */
        {
            double xi = qAsinh(1.0 / eps) / n;
            for (auto& p : poles) {
                p.first *= qSinh(xi) * 0.9;
                p.second *= qCosh(xi) * 0.9;
            }
        }
        break;
    }

    /* Transform to digital domain via bilinear transform */
    auto zPoles = bilinearTransform(poles);

    /* Build transfer function coefficients */
    /* For each conjugate pair, add a second-order section */
    int numSections = (n + 1) / 2;
    coeff.a.resize(n + 1, 0.0);
    coeff.b.resize(n + 1, 0.0);

    /* Start with H(z) = 1, multiply in each section */
    coeff.a[0] = 1.0;
    coeff.b[0] = 1.0;

    int pairIdx = 0;
    for (int i = 0; i < zPoles.size(); i += 2) {
        double pRe = zPoles[i].first;
        double pIm = zPoles[i].second;

        if (i + 1 < zPoles.size() && qAbs(pIm) > 1e-10) {
            /* Conjugate pair */
            double r2 = pRe * pRe + pIm * pIm;
            double sumR = 2.0 * pRe;
            /* (1 - 2*pRe*z^-1 + r2*z^-2) */
            int base = 2 * pairIdx;
            if (base + 2 <= n) {
                coeff.a[base] = 1.0;
                coeff.a[base + 1] = -sumR;
                coeff.a[base + 2] = r2;
                /* Feedforward: place zeros at z=-1 for LP, z=+1 for HP */
                if (type == LowPass) {
                    coeff.b[base] = 1.0;
                    coeff.b[base + 1] = 2.0;
                    coeff.b[base + 2] = 1.0;
                } else {
                    coeff.b[base] = 1.0;
                    coeff.b[base + 1] = -2.0;
                    coeff.b[base + 2] = 1.0;
                }
            }
            pairIdx++;
        } else {
            /* Real pole */
            coeff.a[i] = 1.0;
            coeff.a[i + 1] = -pRe;
            coeff.b[i] = 1.0;
            coeff.b[i + 1] = (type == LowPass) ? 1.0 : -1.0;
        }
    }

    /* Normalize gain: H(1) = 1 for LP, H(-1) = 1 for HP */
    double w = (type == LowPass || type == BandPass) ? 1.0 : -1.0;
    double gainA = 0.0, gainB = 0.0;
    for (int i = 0; i <= n; ++i) {
        gainA += coeff.a[i] * qPow(w, n - i);
        gainB += coeff.b[i] * qPow(w, n - i);
    }
    if (qAbs(gainB) > 1e-15) {
        double gain = gainA / gainB;
        for (double& v : coeff.b) v *= gain;
    }

    m_stats.totalDesigns++;
    m_stats.filterOrder = n;
    m_stats.numCoeffs = coeff.b.size() + coeff.a.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designCompleted(n, coeff.b.size(), coeff.a.size());
    return coeff;
}

/* ---- Frequency response ---- */

QVector<QPair<double, double>> FilterDesign::frequencyResponse(
    const Coefficients& coeff, int numPoints) const
{
    QVector<QPair<double, double>> response;
    int nb = coeff.b.size();
    int na = coeff.a.size();

    for (int i = 0; i < numPoints; ++i) {
        double freq = i * m_fs / (2.0 * numPoints);
        double omega = 2.0 * M_PI * freq / m_fs;

        /* Evaluate H(e^jω) = Σb[k]*e^(-jωk) / Σa[k]*e^(-jωk) */
        double numRe = 0.0, numIm = 0.0;
        for (int k = 0; k < nb; ++k) {
            double angle = -omega * k;
            numRe += coeff.b[k] * qCos(angle);
            numIm += coeff.b[k] * qSin(angle);
        }

        double denRe = 0.0, denIm = 0.0;
        for (int k = 0; k < na; ++k) {
            double angle = -omega * k;
            denRe += coeff.a[k] * qCos(angle);
            denIm += coeff.a[k] * qSin(angle);
        }

        /* H = num/den */
        double denMag2 = denRe * denRe + denIm * denIm;
        double hRe = (numRe * denRe + numIm * denIm) / denMag2;
        double hIm = (numIm * denRe - numRe * denIm) / denMag2;
        double mag = qSqrt(hRe * hRe + hIm * hIm);

        response.append({freq, 20.0 * qLn(mag + 1e-30) / qLn(10.0)});
    }
    return response;
}

/* ---- Apply FIR filter ---- */

QVector<double> FilterDesign::applyFIR(const QVector<double>& input,
                                        const Coefficients& coeff) const
{
    int n = input.size();
    int nb = coeff.b.size();
    if (n == 0) return {};

    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < nb && k <= i; ++k)
            sum += coeff.b[k] * input[i - k];
        output[i] = sum;
    }
    return output;
}

void FilterDesign::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
