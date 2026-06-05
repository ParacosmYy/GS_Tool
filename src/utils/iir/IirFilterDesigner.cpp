/**
 * @file IirFilterDesigner.cpp
 * @brief IIR滤波器设计器实现 — 双二阶级联
 */

#include "utils/iir/IirFilterDesigner.h"

#include <QtMath>
#include <QElapsedTimer>
#include <complex>
#include <algorithm>

IirFilterDesigner::IirFilterDesigner(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<IirFilterDesigner::Biquad> IirFilterDesigner::design(
    FilterType type, Approximation approx, int order,
    double cutoffNorm, double rippleDb) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<std::complex<double>> poles;
    switch (approx) {
    case Approximation::Butterworth:
        poles = butterworthPoles(order); break;
    case Approximation::ChebyshevType1:
        poles = chebyshev1Poles(order, rippleDb); break;
    case Approximation::ChebyshevType2:
        poles = chebyshev1Poles(order, rippleDb); break;
    }

    QVector<Biquad> sections = polesToBiquads(poles, type, cutoffNorm);

    m_stats.totalDesigns++;
    m_stats.totalSectionsGenerated += sections.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designCompleted(order, sections.size());
    return sections;
}

QVector<std::complex<double>> IirFilterDesigner::butterworthPoles(int order) const
{
    QVector<std::complex<double>> poles;
    for (int k = 0; k < order; ++k) {
        double angle = M_PI * (2.0 * k + order + 1) / (2.0 * order);
        poles.append({qCos(angle), qSin(angle)});
    }
    return poles;
}

QVector<std::complex<double>> IirFilterDesigner::chebyshev1Poles(
    int order, double rippleDb) const
{
    double eps = qSqrt(qPow(10.0, rippleDb / 10.0) - 1.0);
    double gamma = qAsinh(1.0 / eps) / order;

    QVector<std::complex<double>> poles;
    for (int k = 0; k < order; ++k) {
        double angle = M_PI * (2.0 * k + 1) / (2.0 * order);
        double re = -qSinh(gamma) * qSin(angle);
        double im = qCosh(gamma) * qCos(angle);
        poles.append({re, im});
    }
    return poles;
}

QVector<IirFilterDesigner::Biquad> IirFilterDesigner::polesToBiquads(
    const QVector<std::complex<double>>& poles, FilterType type,
    double cutoffNorm) const
{
    QVector<Biquad> sections;
    double wc = qTan(M_PI * cutoffNorm);

    for (int i = 0; i < poles.size(); i += 2) {
        Biquad bq;

        if (i + 1 < poles.size()) {
            /* 共轭极点对 */
            std::complex<double> p1 = poles[i] * wc;
            std::complex<double> p2 = std::conj(poles[i]) * wc;

            double a1coeff = -2.0 * p1.real();
            double a2coeff = std::norm(p1);

            /* 归一化 */
            double norm = 1.0 + a1coeff + a2coeff;
            bq.a1 = -a1coeff / norm;
            bq.a2 = -a2coeff / norm;

            switch (type) {
            case FilterType::LowPass:
                bq.b0 = 1.0 / norm;
                bq.b1 = 2.0 / norm;
                bq.b2 = 1.0 / norm;
                break;
            case FilterType::HighPass: {
                double hp = 1.0 - a1coeff + a2coeff;
                bq.b0 = 1.0 / norm;
                bq.b1 = -2.0 / norm;
                bq.b2 = 1.0 / norm;
                double scale = 1.0 / (qAbs(hp) > 1e-12 ? hp : 1.0);
                bq.b0 *= scale; bq.b1 *= scale; bq.b2 *= scale;
                break;
            }
            default:
                bq.b0 = 1.0 / norm;
                bq.b1 = 2.0 / norm;
                bq.b2 = 1.0 / norm;
                break;
            }
        } else {
            /* 奇数阶：实极点 */
            double p = poles[i].real() * wc;
            double norm = 1.0 + p;
            bq.a1 = -(1.0 - p) / norm;
            bq.a2 = 0.0;
            bq.b0 = 1.0 / norm;
            bq.b1 = (type == FilterType::HighPass) ? -1.0 / norm : 1.0 / norm;
            bq.b2 = 0.0;
        }
        sections.append(bq);
    }
    return sections;
}

QVector<QPair<double, double>> IirFilterDesigner::frequencyResponse(
    const QVector<Biquad>& sections, int numPoints) const
{
    QVector<QPair<double, double>> response;
    response.reserve(numPoints);

    for (int k = 0; k < numPoints; ++k) {
        double freq = static_cast<double>(k) / numPoints;
        double angle = 2.0 * M_PI * freq;
        std::complex<double> z1(std::cos(angle), -std::sin(angle));
        std::complex<double> z2(std::cos(2.0 * angle), -std::sin(2.0 * angle));

        std::complex<double> totalGain(1.0, 0.0);
        for (const auto& s : sections) {
            std::complex<double> num = s.b0 + s.b1 * z1 + s.b2 * z2;
            std::complex<double> den = 1.0 + s.a1 * z1 + s.a2 * z2;
            totalGain *= (std::abs(den) > 1e-15) ? num / den : num;
        }
        double db = 20.0 * std::log10(std::max(std::abs(totalGain), 1e-15));
        response.append({freq, db});
    }
    return response;
}

void IirFilterDesigner::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
