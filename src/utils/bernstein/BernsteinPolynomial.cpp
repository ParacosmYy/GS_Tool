#include "utils/bernstein/BernsteinPolynomial.h"
#include <QElapsedTimer>
#include <QtMath>

BernsteinPolynomial::BernsteinPolynomial(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double BernsteinPolynomial::basis(double t, int i, int n) const {
    double coeff = binomial(n, i);
    return coeff * qPow(t, i) * qPow(1.0 - t, n - i);
}

QVector<double> BernsteinPolynomial::evaluate(double t, int degree) const {
    QVector<double> vals(degree + 1);
    for (int i = 0; i <= degree; ++i)
        vals[i] = basis(t, i, degree);
    return vals;
}

QVector<double> BernsteinPolynomial::approximate(
    const QVector<double>& data, int degree) {
    QElapsedTimer timer; timer.start();
    int n = data.size();
    QVector<double> control(degree + 1, 0.0);
    for (int j = 0; j <= degree; ++j) {
        double num = 0.0, den = 0.0;
        for (int i = 0; i < n; ++i) {
            double t = static_cast<double>(i) / static_cast<double>(n - 1);
            double b = basis(t, j, degree);
            num += b * data[i];
            den += b * b;
        }
        control[j] = (den > 1e-15) ? num / den : 0.0;
    }
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalEvaluations;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalEvaluations);
    emit evaluationCompleted(control.isEmpty() ? 0.0 : control[0]);
    return control;
}

QVector<double> BernsteinPolynomial::reconstruct(
    const QVector<double>& cp, int nSamples) const {
    QVector<double> curve(nSamples);
    int deg = cp.size() - 1;
    for (int s = 0; s < nSamples; ++s) {
        double t = static_cast<double>(s) / static_cast<double>(nSamples - 1);
        double val = 0.0;
        for (int i = 0; i <= deg; ++i)
            val += cp[i] * basis(t, i, deg);
        curve[s] = val;
    }
    return curve;
}

double BernsteinPolynomial::binomial(int n, int k) const {
    if (k < 0 || k > n) return 0.0;
    if (k == 0 || k == n) return 1.0;
    double result = 1.0;
    for (int i = 0; i < k; ++i)
        result *= static_cast<double>(n - i) / static_cast<double>(i + 1);
    return result;
}

void BernsteinPolynomial::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
