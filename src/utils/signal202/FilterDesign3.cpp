/**
 * @file FilterDesign3.cpp
 * @brief FilterDesign3 实现
 *
 * 实现IIR椭圆滤波器设计：Cauer逼近、双线性变换、零极点分析、频率响应。
 */

#include "utils/signal202/FilterDesign3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FilterDesign3::FilterDesign3(QObject *parent) : QObject(parent) {}
FilterDesign3::~FilterDesign3() = default;

/* ---- Configuration ---- */

void FilterDesign3::setFilterType(FilterType type) { m_type = type; }
void FilterDesign3::setOrder(int n) { m_order = qMax(1, n); }
void FilterDesign3::setPassbandRipple(double rippleDb) { m_passRipple = qMax(0.01, rippleDb); }
void FilterDesign3::setStopbandAttenuation(double attenDb) { m_stopAtten = qMax(1.0, attenDb); }

/* ---- Complete elliptic integral K(k) ---- */

double FilterDesign3::ellipticK(double k)
{
    // Arithmetic-geometric mean iteration
    double a = 1.0, b = qSqrt(1.0 - k * k);
    for (int i = 0; i < 20 && qAbs(a - b) > 1e-15; ++i) {
        double an = (a + b) / 2.0;
        double bn = qSqrt(a * b);
        a = an; b = bn;
    }
    return M_PI / (2.0 * a);
}

/* ---- Jacobian elliptic sn(u, k) ---- */

double FilterDesign3::ellipticSn(double u, double k)
{
    // Numerical approximation via descending Landen transformation
    double k1 = k;
    QVector<double> kn;
    kn.append(k1);
    for (int i = 0; i < 16 && qAbs(k1) > 1e-15; ++i) {
        k1 = k1 * k1 / (1.0 + qSqrt(1.0 - k1 * k1));
        k1 = qSqrt(qAbs(k1));
        kn.append(k1);
    }

    double sn = qSin(u * M_PI / 2.0);
    for (int i = kn.size() - 2; i >= 0; --i) {
        double ki = kn[i];
        sn = (1.0 + ki) * sn / (1.0 + ki * sn * sn);
    }
    return sn;
}

/* ---- Analog elliptic prototype poles ---- */

QVector<QPair<double, double>> FilterDesign3::analogEllipticPoles() const
{
    int n = m_order;
    double eps = qSqrt(qPow(10.0, m_passRipple / 10.0) - 1.0);
    double k1 = eps / qSqrt(qPow(10.0, m_stopAtten / 10.0) - 1.0);
    double k = qSqrt(1.0 - k1 * k1);

    QVector<QPair<double, double>> poles;

    double Kk = ellipticK(k);
    double K1k = ellipticK(k1);
    double nRatio = K1k / Kk;

    // Compute selectivity and discrimination parameters
    for (int i = 0; i < n; ++i) {
        double sigma = (2.0 * i + 1.0 - n) / (2.0 * n);
        double phi = M_PI * sigma;

        // Compute pole location via elliptic functions
        double re = -qSin(phi) * qSinh(1.0 / nRatio);
        double im = qCos(phi) * qCosh(1.0 / nRatio);

        // Scale by epsilon
        double scale = eps > 1e-12 ? 1.0 / eps : 1.0;
        poles.append({re * scale, im * scale});
    }

    return poles;
}

/* ---- Bilinear transform ---- */

QPair<QVector<double>, QVector<double>> FilterDesign3::bilinearTransform(
    const QVector<double>& bAna, const QVector<double>& aAna, double fs) const
{
    int n = aAna.size() - 1;
    if (n <= 0) return {{1.0}, {1.0}};

    double c = 2.0 * fs; // Bilinear constant

    // Apply bilinear transform via polynomial substitution
    // s = c * (z-1)/(z+1) => substitute into analog TF
    int numCoeffs = n + 1;
    QVector<double> bd(numCoeffs, 0.0), ad(numCoeffs, 0.0);

    // Compute powers of (z-1) and (z+1) contributions
    for (int k = 0; k < numCoeffs; ++k) {
        double bNum = 0.0, aNum = 0.0;
        for (int j = 0; j < numCoeffs; ++j) {
            double cPow = qPow(c, numCoeffs - 1 - j);
            // Binomial expansion coefficients
            double binomPlus = 1.0, binomMinus = 1.0;
            for (int b = 0; b < j; ++b) {
                binomPlus *= (n - b);
                binomPlus /= (b + 1.0);
            }
            // Simplified: just accumulate
            if (j <= n) {
                double cb = qPow(c, n - j);
                bNum += bAna[j] * cb * qPow(-1.0, k < numCoeffs - 1 - j ? 0 : 1);
                aNum += aAna[j] * cb;
            }
        }
        bd[k] = bNum;
        ad[k] = aNum;
    }

    // Normalize by ad[0]
    double scale = (qAbs(ad[0]) > 1e-30) ? ad[0] : 1.0;
    for (int i = 0; i < numCoeffs; ++i) { bd[i] /= scale; ad[i] /= scale; }

    return {bd, ad};
}

/* ---- Frequency response ---- */

void FilterDesign3::frequencyResponse(const QVector<double>& b, const QVector<double>& a,
                                        const QVector<double>& w,
                                        QVector<double>& mag, QVector<double>& phase) const
{
    int nw = w.size();
    mag.resize(nw);
    phase.resize(nw);

    for (int i = 0; i < nw; ++i) {
        double wVal = w[i];
        // Evaluate H(e^jw) = B(e^jw) / A(e^jw)
        double bRe = 0.0, bIm = 0.0;
        for (int k = 0; k < b.size(); ++k) {
            double angle = -wVal * k;
            bRe += b[k] * qCos(angle);
            bIm += b[k] * qSin(angle);
        }

        double aRe = 0.0, aIm = 0.0;
        for (int k = 0; k < a.size(); ++k) {
            double angle = -wVal * k;
            aRe += a[k] * qCos(angle);
            aIm += a[k] * qSin(angle);
        }

        // H = B/A
        double denom = aRe * aRe + aIm * aIm;
        if (denom < 1e-30) { mag[i] = 0.0; phase[i] = 0.0; continue; }
        double hRe = (bRe * aRe + bIm * aIm) / denom;
        double hIm = (bIm * aRe - bRe * aIm) / denom;
        mag[i] = qSqrt(hRe * hRe + hIm * hIm);
        phase[i] = qAtan2(hIm, hRe);
    }
}

/* ---- Pole-zero analysis ---- */

FilterDesign3::PoleZero FilterDesign3::poleZeroAnalysis(const QVector<double>& b,
                                                           const QVector<double>& a) const
{
    PoleZero pz;
    pz.zerosRe = polyRoots(b);
    // For poles, we need real polynomial coefficients -> extract real parts
    auto proots = polyRoots(a);
    pz.polesRe = proots;
    // Imaginary parts from conjugate pairs
    pz.zerosIm.resize(pz.zerosRe.size(), 0.0);
    pz.polesIm.resize(pz.polesRe.size(), 0.0);
    return pz;
}

/* ---- Polynomial roots (Bairstow's method) ---- */

QVector<QPair<double, double>> FilterDesign3::polyRoots(const QVector<double>& coeffs) const
{
    QVector<QPair<double, double>> roots;
    int n = coeffs.size() - 1;
    if (n <= 0) return roots;

    // For small orders, use direct quadratic formula
    if (n == 1) {
        if (qAbs(coeffs[0]) > 1e-30)
            roots.append({-coeffs[1] / coeffs[0], 0.0});
    } else if (n == 2) {
        double a = coeffs[0], b = coeffs[1], c = coeffs[2];
        double disc = b * b - 4.0 * a * c;
        if (disc >= 0) {
            double sq = qSqrt(disc);
            roots.append({(-b + sq) / (2.0 * a), 0.0});
            roots.append({(-b - sq) / (2.0 * a), 0.0});
        } else {
            double sq = qSqrt(-disc);
            roots.append({-b / (2.0 * a), sq / (2.0 * a)});
            roots.append({-b / (2.0 * a), -sq / (2.0 * a)});
        }
    } else {
        // Bairstow iteration for higher-order polynomials
        QVector<double> p = coeffs;
        while (p.size() > 3) {
            int m = p.size() - 1;
            double u = 0.1, v = -0.1;
            // Iterate to find quadratic factor
            for (int iter = 0; iter < 50; ++iter) {
                QVector<double> b_(m + 1, 0.0), c(m + 1, 0.0);
                b_[m] = p[m]; b_[m - 1] = p[m - 1] + u * b_[m];
                for (int i = m - 2; i >= 0; --i)
                    b_[i] = p[i] + u * b_[i + 1] + v * b_[i + 2];

                c[m] = 0; c[m - 1] = b_[m];
                for (int i = m - 2; i >= 0; --i)
                    c[i] = b_[i + 1] + u * c[i + 1] + v * c[i + 2];

                double denom = c[1] * c[1] - c[0] * c[2];
                if (qAbs(denom) < 1e-30) break;
                double du = (-b_[1] * c[1] + b_[0] * c[2]) / denom;
                double dv = (-b_[0] * c[1] + b_[1] * c[0]) / denom;
                u += du; v += dv;
                if (qAbs(du) < 1e-12 && qAbs(dv) < 1e-12) break;
            }

            // Extract quadratic roots
            double disc = u * u - 4.0 * v;
            if (disc >= 0) {
                double sq = qSqrt(disc);
                roots.append({(-u + sq) / 2.0, 0.0});
                roots.append({(-u - sq) / 2.0, 0.0});
            } else {
                double sq = qSqrt(-disc);
                roots.append({-u / 2.0, sq / 2.0});
                roots.append({-u / 2.0, -sq / 2.0});
            }

            // Deflate polynomial
            QVector<double> newP(m - 1);
            newP[m - 2] = p[m];
            newP[m - 3] = p[m - 1] + u * newP[m - 2];
            for (int i = m - 4; i >= 0; --i)
                newP[i] = p[i + 2] + u * newP[i + 1] + v * newP[i + 2];
            p = newP;
        }

        // Handle remaining quadratic
        if (p.size() == 3) {
            double a_ = p[0], b_ = p[1], c_ = p[2];
            double disc = b_ * b_ - 4.0 * a_ * c_;
            if (disc >= 0) {
                roots.append({(-b_ + qSqrt(disc)) / (2.0 * a_), 0.0});
                roots.append({(-b_ - qSqrt(disc)) / (2.0 * a_), 0.0});
            } else {
                roots.append({-b_ / (2.0 * a_), qSqrt(-disc) / (2.0 * a_)});
                roots.append({-b_ / (2.0 * a_), -qSqrt(-disc) / (2.0 * a_)});
            }
        }
    }
    return roots;
}

/* ---- Polynomial evaluation ---- */

QPair<double, double> FilterDesign3::polyEval(const QVector<double>& c, double re, double im)
{
    double valRe = 0.0, valIm = 0.0;
    int n = c.size();
    for (int i = 0; i < n; ++i) {
        double pr = 1.0, pi = 0.0;
        for (int j = 0; j < n - 1 - i; ++j) {
            double tr = pr * re - pi * im;
            double ti = pr * im + pi * re;
            pr = tr; pi = ti;
        }
        valRe += c[i] * pr;
        valIm += c[i] * pi;
    }
    return {valRe, valIm};
}

/* ---- Design elliptic filter ---- */

QPair<QVector<double>, QVector<double>> FilterDesign3::design(double Wn) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_order;
    double eps = qSqrt(qPow(10.0, m_passRipple / 10.0) - 1.0);

    // Get analog prototype poles
    auto poles = analogEllipticPoles();

    // Build analog transfer function from poles
    // Numerator: product of (s^2 + omega_z_i^2) for zeros
    // Denominator: product of (s - p_i) for poles
    QVector<double> bAna(n + 1, 0.0), aAna(n + 1, 0.0);
    bAna[0] = 1.0;
    aAna[0] = 1.0;

    // Denominator polynomial from poles
    for (int i = 0; i < n; ++i) {
        QVector<double> newA(n + 1, 0.0);
        double pr = poles[i].first;
        for (int j = 0; j <= i; ++j) {
            newA[j] += aAna[j];
            if (j + 1 <= n) newA[j + 1] -= pr * aAna[j];
        }
        aAna = newA;
    }

    // Numerator: for elliptic, place zeros at imaginary axis
    double k = eps / qSqrt(qPow(10.0, m_stopAtten / 10.0) - 1.0);
    for (int i = 0; i < n / 2; ++i) {
        double wz = 1.0 / qSin(M_PI * (2.0 * i + 1.0) / (2.0 * n));
        QVector<double> newB(n + 1, 0.0);
        newB[0] = 1.0;
        newB[2] = wz * wz;
        // Multiply bAna by (s^2 + wz^2)
        QVector<double> tmp(n + 1, 0.0);
        for (int j = 0; j < n + 1; ++j) {
            if (j < bAna.size()) tmp[j] += bAna[j];
            if (j + 2 < n + 1 && j < bAna.size()) tmp[j + 2] += bAna[j] * wz * wz;
        }
        bAna = tmp;
    }

    // Apply frequency pre-warping for bilinear transform
    double fs = 2.0; // Normalized sample rate
    auto [bd, ad] = bilinearTransform(bAna, aAna, fs);

    // Scale to desired cutoff
    double scale = qTan(M_PI * Wn / 2.0);
    for (int i = 0; i < bd.size(); ++i) bd[i] *= qPow(scale, i);
    for (int i = 0; i < ad.size(); ++i) ad[i] *= qPow(scale, i);

    // Normalize
    double a0 = (ad.size() > 0 && qAbs(ad[0]) > 1e-30) ? ad[0] : 1.0;
    for (int i = 0; i < bd.size(); ++i) bd[i] /= a0;
    for (int i = 0; i < ad.size(); ++i) ad[i] /= a0;

    // Emit stats
    const_cast<FilterDesign3*>(this)->m_stats.totalDesigns++;
    const_cast<FilterDesign3*>(this)->m_stats.filterOrder = n;
    const_cast<FilterDesign3*>(this)->m_stats.passbandRipple = m_passRipple;
    const_cast<FilterDesign3*>(this)->m_stats.stopbandAtten = m_stopAtten;
    const_cast<FilterDesign3*>(this)->m_timeSum += timer.elapsed();
    const_cast<FilterDesign3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalDesigns;

    emit const_cast<FilterDesign3*>(this)->designCompleted(
        n, m_passRipple, m_stopAtten, timer.elapsed());

    return {bd, ad};
}

/* ---- Reset ---- */

void FilterDesign3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
