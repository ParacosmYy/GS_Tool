/**
 * @file LinearPredictiveCoder.cpp
 * @brief 线性预测编码器实现 — Levinson-Durbin递推
 */

#include "utils/lpc/LinearPredictiveCoder.h"

#include <QtMath>
#include <QElapsedTimer>

LinearPredictiveCoder::LinearPredictiveCoder(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

LinearPredictiveCoder::LpcResult LinearPredictiveCoder::analyze(
    const QVector<double>& data, int order)
{
    LpcResult result;
    if (data.size() < order + 1 || order < 1) return result;

    QElapsedTimer timer;
    timer.start();

    QVector<double> r = autocorrelation(data, order);
    if (r.isEmpty() || qAbs(r[0]) < 1e-15) return result;

    double error = 0.0;
    QVector<double> a;
    if (!levinsonDurbin(r, order, a, error)) return result;

    result.coefficients = a;
    result.residualEnergy = error;
    result.predictionGain = (error > 0 && r[0] > 0)
        ? 10.0 * std::log10(r[0] / error) : 0.0;
    result.reflectionFirst = a.isEmpty() ? 0.0 : a[0];

    m_stats.totalAnalyses++;
    m_stats.totalCoefficientsComputed += order;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(order, result.predictionGain);
    return result;
}

QVector<double> LinearPredictiveCoder::synthesize(
    const QVector<double>& residual, const QVector<double>& coefficients) const
{
    int n = residual.size();
    int p = coefficients.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        output[i] = residual[i];
        for (int j = 0; j < p && j < i; ++j) {
            output[i] -= coefficients[j] * output[i - 1 - j];
        }
    }
    return output;
}

QVector<double> LinearPredictiveCoder::reflectionCoefficients(
    const QVector<double>& data, int maxOrder) const
{
    QVector<double> refl;
    if (data.size() < maxOrder + 1) return refl;

    QVector<double> r = autocorrelation(data, maxOrder);
    if (r.isEmpty() || qAbs(r[0]) < 1e-15) return refl;

    double error = r[0];
    QVector<double> a(maxOrder, 0.0);

    for (int m = 0; m < maxOrder; ++m) {
        double km = -r[m + 1];
        for (int j = 0; j < m; ++j) km -= a[j] * r[m - j];
        km /= (qAbs(error) > 1e-15) ? error : 1.0;

        refl.append(km);

        QVector<double> aNew = a;
        for (int j = 0; j < m; ++j) {
            aNew[j] = a[j] + km * a[m - 1 - j];
        }
        aNew[m] = km;
        a = aNew;
        error *= (1.0 - km * km);
    }
    return refl;
}

QVector<QPair<double, double>> LinearPredictiveCoder::spectralEnvelope(
    const QVector<double>& coefficients, int numPoints) const
{
    QVector<QPair<double, double>> envelope;
    envelope.reserve(numPoints);

    int p = coefficients.size();
    for (int k = 0; k < numPoints; ++k) {
        double freq = static_cast<double>(k) / numPoints;
        double real = 1.0, imag = 0.0;
        for (int j = 0; j < p; ++j) {
            double angle = -2.0 * M_PI * freq * (j + 1);
            real += coefficients[j] * qCos(angle);
            imag += coefficients[j] * qSin(angle);
        }
        double mag = qSqrt(real * real + imag * imag);
        double db = (mag > 1e-15) ? -20.0 * std::log10(mag) : 120.0;
        envelope.append({freq, db});
    }
    return envelope;
}

QVector<double> LinearPredictiveCoder::autocorrelation(
    const QVector<double>& data, int maxLag)
{
    int n = data.size();
    QVector<double> r(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i) {
            sum += data[i] * data[i + lag];
        }
        r[lag] = sum;
    }
    return r;
}

bool LinearPredictiveCoder::levinsonDurbin(const QVector<double>& r, int order,
                                            QVector<double>& a, double& error)
{
    if (qAbs(r[0]) < 1e-15) return false;

    error = r[0];
    a.resize(order);
    a.fill(0.0);

    QVector<double> aPrev(order, 0.0);

    for (int m = 0; m < order; ++m) {
        double km = -r[m + 1];
        for (int j = 0; j < m; ++j) km -= aPrev[j] * r[m - j];
        km /= (qAbs(error) > 1e-15) ? error : 1.0;

        a[m] = km;
        for (int j = 0; j < m; ++j) {
            a[j] = aPrev[j] + km * aPrev[m - 1 - j];
        }
        aPrev = a;

        error *= (1.0 - km * km);
        if (error <= 0) return false;
    }
    return true;
}

void LinearPredictiveCoder::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
