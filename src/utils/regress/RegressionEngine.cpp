/**
 * @file RegressionEngine.cpp
 * @brief 回归引擎实现
 */

#include "utils/regress/RegressionEngine.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

RegressionEngine::RegressionEngine(QObject* parent)
    : QObject(parent), m_polyDegree(2), m_timeSum(0.0) {}

void RegressionEngine::setPolynomialDegree(int degree) { m_polyDegree = qMax(1, degree); }

RegressionEngine::RegressionResult RegressionEngine::fit(
    const QVector<double>& x, const QVector<double>& y, RegressionType type)
{
    QElapsedTimer timer;
    timer.start();

    RegressionResult result;
    if (x.size() != y.size() || x.size() < 2) return result;

    switch (type) {
    case RegressionType::Linear:     result = linearRegression(x, y); break;
    case RegressionType::Polynomial: result = polyRegression(x, y); break;
    case RegressionType::Logarithmic: result = logRegression(x, y); break;
    case RegressionType::Exponential: result = expRegression(x, y); break;
    case RegressionType::Power:      result = powerRegression(x, y); break;
    }

    /* 计算评估指标 */
    double ssRes = 0.0, ssTot = 0.0, absErr = 0.0;
    double yMean = 0.0;
    for (double v : y) yMean += v;
    yMean /= y.size();

    for (int i = 0; i < x.size(); ++i) {
        double predicted = predict(result, x[i]);
        double residual = y[i] - predicted;
        ssRes += residual * residual;
        ssTot += (y[i] - yMean) * (y[i] - yMean);
        absErr += qAbs(residual);
    }

    int n = x.size();
    int p = result.coefficients.size();
    result.rmse = qSqrt(ssRes / n);
    result.mae = absErr / n;
    result.rSquared = (ssTot > 0.0) ? 1.0 - ssRes / ssTot : 0.0;
    result.adjustedRSquared = (n > p && p > 0) ? 1.0 - (1.0 - result.rSquared) * (n - 1) / (n - p) : result.rSquared;

    double elapsed = timer.elapsed();
    ++m_stats.totalRegressions;
    double rSum = m_stats.avgRSquared * (m_stats.totalRegressions - 1) + result.rSquared;
    m_stats.avgRSquared = rSum / m_stats.totalRegressions;
    double rmseSum = m_stats.avgRmse * (m_stats.totalRegressions - 1) + result.rmse;
    m_stats.avgRmse = rmseSum / m_stats.totalRegressions;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalRegressions;

    emit regressionComplete(result);
    return result;
}

RegressionEngine::RegressionResult RegressionEngine::autoFit(const QVector<double>& x, const QVector<double>& y)
{
    QList<QPair<RegressionType, RegressionResult>> candidates;
    candidates.append({RegressionType::Linear, fit(x, y, RegressionType::Linear)});
    candidates.append({RegressionType::Polynomial, fit(x, y, RegressionType::Polynomial)});
    candidates.append({RegressionType::Logarithmic, fit(x, y, RegressionType::Logarithmic)});
    candidates.append({RegressionType::Exponential, fit(x, y, RegressionType::Exponential)});
    candidates.append({RegressionType::Power, fit(x, y, RegressionType::Power)});

    RegressionResult best = candidates[0].second;
    RegressionType bestType = candidates[0].first;
    for (const auto& c : candidates) {
        if (c.second.rSquared > best.rSquared) { best = c.second; bestType = c.first; }
    }

    emit modelSelected(bestType, best.rSquared);
    return best;
}

double RegressionEngine::predict(const RegressionResult& model, double x) const
{
    switch (model.type) {
    case RegressionType::Linear:
    case RegressionType::Polynomial: {
        double y = 0.0;
        double xPow = 1.0;
        for (int i = 0; i < model.coefficients.size(); ++i) {
            y += model.coefficients[i] * xPow;
            xPow *= x;
        }
        return y;
    }
    case RegressionType::Logarithmic:
        return (x > 0.0) ? model.coefficients[0] * qLn(x) + model.coefficients[1] : 0.0;
    case RegressionType::Exponential:
        return model.coefficients[0] * qExp(model.coefficients[1] * x);
    case RegressionType::Power:
        return (x > 0.0) ? model.coefficients[0] * qPow(x, model.coefficients[1]) : 0.0;
    }
    return 0.0;
}

RegressionEngine::RegressionResult RegressionEngine::linearRegression(const QVector<double>& x, const QVector<double>& y)
{
    RegressionResult result;
    result.type = RegressionType::Linear;

    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    int n = x.size();
    for (int i = 0; i < n; ++i) {
        sumX += x[i]; sumY += y[i];
        sumXY += x[i] * y[i]; sumX2 += x[i] * x[i];
    }

    double denom = n * sumX2 - sumX * sumX;
    if (qAbs(denom) < 1e-15) {
        result.coefficients = {0.0, sumY / n};
    } else {
        result.slope = (n * sumXY - sumX * sumY) / denom;
        result.intercept = (sumY - result.slope * sumX) / n;
        result.coefficients = {result.slope, result.intercept};
    }
    return result;
}

RegressionEngine::RegressionResult RegressionEngine::polyRegression(const QVector<double>& x, const QVector<double>& y)
{
    RegressionResult result;
    result.type = RegressionType::Polynomial;
    int n = x.size();
    int deg = qMin(m_polyDegree, n - 1);
    int m = deg + 1;

    /* 正规方程: (X^T X) β = X^T y */
    QVector<QVector<double>> mat(m, QVector<double>(m, 0.0));
    QVector<double> rhs(m, 0.0);

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) sum += qPow(x[k], i + j);
            mat[i][j] = sum;
        }
        double sum = 0.0;
        for (int k = 0; k < n; ++k) sum += y[k] * qPow(x[k], i);
        rhs[i] = sum;
    }

    /* 高斯消元 */
    for (int col = 0; col < m; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < m; ++row)
            if (qAbs(mat[row][col]) > qAbs(mat[maxRow][col])) maxRow = row;
        std::swap(mat[col], mat[maxRow]);
        std::swap(rhs[col], rhs[maxRow]);
        if (qAbs(mat[col][col]) < 1e-15) continue;
        for (int row = col + 1; row < m; ++row) {
            double factor = mat[row][col] / mat[col][col];
            for (int j = col; j < m; ++j) mat[row][j] -= factor * mat[col][j];
            rhs[row] -= factor * rhs[col];
        }
    }

    /* 回代 */
    result.coefficients.resize(m);
    for (int i = m - 1; i >= 0; --i) {
        double sum = rhs[i];
        for (int j = i + 1; j < m; ++j) sum -= mat[i][j] * result.coefficients[j];
        result.coefficients[i] = (qAbs(mat[i][i]) > 1e-15) ? sum / mat[i][i] : 0.0;
    }
    return result;
}

RegressionEngine::RegressionResult RegressionEngine::logRegression(const QVector<double>& x, const QVector<double>& y)
{
    RegressionResult result;
    result.type = RegressionType::Logarithmic;

    QVector<double> lnX;
    for (double v : x) lnX.append((v > 0.0) ? qLn(v) : -20.0);

    auto linResult = linearRegression(lnX, y);
    result.coefficients = linResult.coefficients;
    return result;
}

RegressionEngine::RegressionResult RegressionEngine::expRegression(const QVector<double>& x, const QVector<double>& y)
{
    RegressionResult result;
    result.type = RegressionType::Exponential;

    QVector<double> lnY;
    for (double v : y) lnY.append((v > 0.0) ? qLn(v) : -20.0);

    auto linResult = linearRegression(x, lnY);
    result.coefficients = {qExp(linResult.coefficients[1]), linResult.coefficients[0]};
    return result;
}

RegressionEngine::RegressionResult RegressionEngine::powerRegression(const QVector<double>& x, const QVector<double>& y)
{
    RegressionResult result;
    result.type = RegressionType::Power;

    QVector<double> lnX, lnY;
    for (double v : x) lnX.append((v > 0.0) ? qLn(v) : -20.0);
    for (double v : y) lnY.append((v > 0.0) ? qLn(v) : -20.0);

    auto linResult = linearRegression(lnX, lnY);
    result.coefficients = {qExp(linResult.coefficients[1]), linResult.coefficients[0]};
    return result;
}

void RegressionEngine::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
