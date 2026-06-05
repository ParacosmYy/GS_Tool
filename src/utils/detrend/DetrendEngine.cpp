/**
 * @file DetrendEngine.cpp
 * @brief 去趋势引擎实现
 */

#include "utils/detrend/DetrendEngine.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

DetrendEngine::DetrendEngine(QObject* parent)
    : QObject(parent), m_method(Method::Linear), m_polyDegree(2),
      m_windowSize(10), m_timeSum(0.0) {}

void DetrendEngine::setMethod(Method m) { m_method = m; }
void DetrendEngine::setPolynomialDegree(int d) { m_polyDegree = qMax(1, d); }
void DetrendEngine::setWindowSize(int s) { m_windowSize = qMax(2, s); }

DetrendEngine::DetrendResult DetrendEngine::detrend(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    DetrendResult result;
    if (data.size() < 2) { result.detrended = data; return result; }

    int n = data.size();
    result.trend.resize(n);
    result.detrended.resize(n);

    /* 计算原始方差 */
    double origSum = 0.0;
    for (double v : data) origSum += v;
    double origMean = origSum / n;
    double origVar = 0.0;
    for (double v : data) { double d = v - origMean; origVar += d * d; }
    origVar /= n;
    result.originalVariance = origVar;

    switch (m_method) {
    case Method::Constant: {
        for (int i = 0; i < n; ++i) {
            result.trend[i] = origMean;
            result.detrended[i] = data[i] - origMean;
        }
        break;
    }
    case Method::Linear: {
        /* 线性回归: y = ax + b */
        double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
        for (int i = 0; i < n; ++i) {
            double x = static_cast<double>(i);
            sumX += x; sumY += data[i]; sumXY += x * data[i]; sumX2 += x * x;
        }
        double denom = n * sumX2 - sumX * sumX;
        double a = (qAbs(denom) > 1e-15) ? (n * sumXY - sumX * sumY) / denom : 0.0;
        double b = (sumY - a * sumX) / n;
        for (int i = 0; i < n; ++i) {
            result.trend[i] = a * i + b;
            result.detrended[i] = data[i] - result.trend[i];
        }
        break;
    }
    case Method::Polynomial: {
        /* 简化: 使用3次多项式最小二乘拟合 */
        int deg = qMin(m_polyDegree, n - 1);
        int m = deg + 1;

        /* 正规方程 */
        QVector<QVector<double>> mat(m, QVector<double>(m, 0.0));
        QVector<double> rhs(m, 0.0);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                double s = 0.0;
                for (int k = 0; k < n; ++k) s += qPow(static_cast<double>(k), i + j);
                mat[i][j] = s;
            }
            double s = 0.0;
            for (int k = 0; k < n; ++k) s += data[k] * qPow(static_cast<double>(k), i);
            rhs[i] = s;
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
                double f = mat[row][col] / mat[col][col];
                for (int j = col; j < m; ++j) mat[row][j] -= f * mat[col][j];
                rhs[row] -= f * rhs[col];
            }
        }
        QVector<double> coeff(m);
        for (int i = m - 1; i >= 0; --i) {
            double s = rhs[i];
            for (int j = i + 1; j < m; ++j) s -= mat[i][j] * coeff[j];
            coeff[i] = (qAbs(mat[i][i]) > 1e-15) ? s / mat[i][i] : 0.0;
        }

        for (int i = 0; i < n; ++i) {
            double y = 0.0, xp = 1.0;
            for (int d = 0; d < m; ++d) { y += coeff[d] * xp; xp *= i; }
            result.trend[i] = y;
            result.detrended[i] = data[i] - y;
        }
        break;
    }
    case Method::MovingAverage: {
        int half = m_windowSize / 2;
        for (int i = 0; i < n; ++i) {
            int start = qMax(0, i - half);
            int end = qMin(n - 1, i + half);
            double sum = 0.0;
            for (int j = start; j <= end; ++j) sum += data[j];
            result.trend[i] = sum / (end - start + 1);
            result.detrended[i] = data[i] - result.trend[i];
        }
        break;
    }
    }

    /* 计算残余方差 */
    double resMean = 0.0;
    for (double v : result.detrended) resMean += v;
    resMean /= n;
    double resVar = 0.0;
    for (double v : result.detrended) { double d = v - resMean; resVar += d * d; }
    resVar /= n;
    result.residualVariance = resVar;
    result.varianceReduction = (origVar > 0.0) ? (1.0 - resVar / origVar) * 100.0 : 0.0;

    double elapsed = timer.elapsed();
    ++m_stats.totalDetrends;
    double sum = m_stats.avgVarianceReduction * (m_stats.totalDetrends - 1) + result.varianceReduction;
    m_stats.avgVarianceReduction = sum / m_stats.totalDetrends;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDetrends;

    emit detrendComplete(result.varianceReduction);
    return result;
}

void DetrendEngine::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
