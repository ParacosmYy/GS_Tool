#include "utils/taylor/TaylorSeries.h"
#include <QElapsedTimer>
#include <QtMath>

TaylorSeries::TaylorSeries(QObject* parent) : QObject(parent), m_timeSum(0.0) {}

double TaylorSeries::approximate(const Func& func, double center,
                                  int order, double x) {
    auto coeffs = coefficients(func, center, order);
    double val = evaluate(coeffs, center, x);
    emit evaluationCompleted(order, val);
    return val;
}

QVector<double> TaylorSeries::coefficients(const Func& func,
                                            double center, int order) {
    QVector<double> c(order + 1);
    double h = 1e-4;
    double factorial = 1.0;

    c[0] = func(center);
    for (int k = 1; k <= order; ++k) {
        factorial *= static_cast<double>(k);
        /* 中心差分 */
        double fPlus = func(center + h);
        double fMinus = func(center - h);
        /* 高阶导数数值近似(递推) */
        double deriv = (fPlus - fMinus) / (2.0 * h);
        c[k] = deriv / factorial;
        h *= 0.5;
    }
    return c;
}

double TaylorSeries::evaluate(const QVector<double>& coeffs,
                               double center, double x) const {
    double dx = x - center;
    double result = 0.0;
    double power = 1.0;
    for (int i = 0; i < coeffs.size(); ++i) {
        result += coeffs[i] * power;
        power *= dx;
    }
    return result;
}

QVector<double> TaylorSeries::sinCoeffs(int order) {
    QVector<double> c(order + 1, 0.0);
    for (int k = 0; k <= order; ++k) {
        int n = k / 2;
        if (k % 2 == 1) {
            double sign = (n % 2 == 0) ? 1.0 : -1.0;
            double fact = 1.0;
            for (int i = 1; i <= k; ++i) fact *= i;
            c[k] = sign / fact;
        }
    }
    return c;
}

QVector<double> TaylorSeries::expCoeffs(int order) {
    QVector<double> c(order + 1);
    double fact = 1.0;
    for (int k = 0; k <= order; ++k) {
        if (k > 0) fact *= k;
        c[k] = 1.0 / fact;
    }
    return c;
}

void TaylorSeries::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
