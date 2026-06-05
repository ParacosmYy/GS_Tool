/**
 * @file DataSmoother.cpp
 * @brief 数据平滑器实现
 */

#include "utils/smoother/DataSmoother.h"
#include <QtMath>

DataSmoother::DataSmoother(QObject* parent)
    : QObject(parent), m_method(SmoothMethod::Exponential)
    , m_windowSize(5), m_alpha(0.3)
    , m_prevOutput(0.0), m_initialized(false) {}

void DataSmoother::setMethod(SmoothMethod m) { m_method = m; }
void DataSmoother::setWindowSize(int s) { m_windowSize = qMax(3, s | 1); }
void DataSmoother::setAlpha(double a) { m_alpha = qBound(0.01, a, 1.0); }

QVector<double> DataSmoother::smooth(const QVector<double>& data)
{
    if (data.size() < 2) return data;

    QVector<double> result;
    switch (m_method) {
    case SmoothMethod::Exponential:
        result = smoothExponential(data); break;
    case SmoothMethod::Gaussian:
        result = smoothGaussian(data); break;
    case SmoothMethod::Triangular:
        result = smoothTriangular(data); break;
    case SmoothMethod::SavitzkyGolay:
        result = smoothGaussian(data); break; /* 简化用高斯近似 */
    }

    m_stats.totalSmoothOps++;
    m_stats.totalPointsProcessed += static_cast<quint64>(data.size());
    emit smoothComplete(data.size());
    return result;
}

double DataSmoother::smoothOne(double value)
{
    if (!m_initialized) {
        m_prevOutput = value;
        m_initialized = true;
        return value;
    }
    m_prevOutput = m_alpha * value + (1.0 - m_alpha) * m_prevOutput;
    ++m_stats.totalPointsProcessed;
    return m_prevOutput;
}

void DataSmoother::resetStatistics() { m_stats = Stats{}; }

QVector<double> DataSmoother::smoothExponential(const QVector<double>& data)
{
    QVector<double> result(data.size());
    result[0] = data[0];
    for (int i = 1; i < data.size(); ++i) {
        result[i] = m_alpha * data[i] + (1.0 - m_alpha) * result[i - 1];
    }
    return result;
}

QVector<double> DataSmoother::smoothGaussian(const QVector<double>& data)
{
    int half = m_windowSize / 2;
    double sigma = half / 2.0;
    QVector<double> weights;
    double wSum = 0.0;
    for (int i = -half; i <= half; ++i) {
        double w = qExp(-0.5 * (i * i) / (sigma * sigma));
        weights.append(w);
        wSum += w;
    }
    for (auto& w : weights) w /= wSum;

    QVector<double> result(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = -half; j <= half; ++j) {
            int idx = qBound(0, i + j, data.size() - 1);
            sum += data[idx] * weights[j + half];
        }
        result[i] = sum;
    }
    return result;
}

QVector<double> DataSmoother::smoothTriangular(const QVector<double>& data)
{
    int half = m_windowSize / 2;
    double wTotal = 0.0;
    for (int j = -half; j <= half; ++j) {
        wTotal += (half + 1 - qAbs(j));
    }

    QVector<double> result(data.size());
    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = -half; j <= half; ++j) {
            int idx = qBound(0, i + j, data.size() - 1);
            double w = (half + 1 - qAbs(j)) / wTotal;
            sum += data[idx] * w;
        }
        result[i] = sum;
    }
    return result;
}
