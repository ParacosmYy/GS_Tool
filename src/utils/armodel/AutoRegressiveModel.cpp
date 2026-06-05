/**
 * @file AutoRegressiveModel.cpp
 * @brief 自回归(AR)模型实现 — Yule-Walker/Levinson-Durbin
 */

#include "utils/armodel/AutoRegressiveModel.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
AutoRegressiveModel::AutoRegressiveModel(QObject* parent)
    : QObject(parent)
    , m_noiseVar(0.0)
    , m_order(0)
    , m_timeSum(0.0) {}

/** @brief 拟合AR模型 @param data 时间序列 @param order 阶数(0=自动) */
void AutoRegressiveModel::fit(const QVector<double>& data, int order)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 4) return;

    int p = order;
    if (p <= 0) {
        p = selectOrder(data, std::min(20, n / 3));
    }
    p = std::min(p, n - 1);
    if (p < 1) p = 1;

    /* 计算自相关 */
    QVector<double> acf = autocorrelation(data, p);
    if (acf.isEmpty() || acf[0] == 0.0) return;

    /* Levinson-Durbin递归求解 */
    auto result = levinsonDurbin(acf, p);
    m_coeffs = result.first;
    m_noiseVar = result.second;
    m_order = p;
    m_lastData = data;

    m_stats.totalFits++;
    m_stats.bestOrder = p;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalFits > 0) ? m_timeSum / m_stats.totalFits : 0.0;

    emit modelFitted(p, m_noiseVar);
}

/** @brief 自动选择最优阶数 @param data 时间序列 @param maxOrder 最大阶数 @return 最优阶数 */
int AutoRegressiveModel::selectOrder(const QVector<double>& data, int maxOrder)
{
    int n = data.size();
    if (n < 4) return 1;

    maxOrder = std::min(maxOrder, n / 3);
    if (maxOrder < 1) maxOrder = 1;

    double bestAicVal = 1e30;
    int bestP = 1;

    for (int p = 1; p <= maxOrder; ++p) {
        double aic = computeAic(data, p);
        if (aic < bestAicVal) {
            bestAicVal = aic;
            bestP = p;
        }
    }

    m_stats.bestAic = bestAicVal;
    m_stats.bestBic = computeBic(data, bestP);
    return bestP;
}

/** @brief 向前预测 @param steps 预测步数 @return 预测值 */
QVector<double> AutoRegressiveModel::predict(int steps)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> predictions;
    if (m_coeffs.isEmpty() || m_lastData.isEmpty() || steps <= 0) {
        return predictions;
    }

    predictions.reserve(steps);
    int n = m_lastData.size();
    int p = m_coeffs.size();

    /* 复制尾部数据作为初始窗口 */
    QVector<double> window;
    for (int i = std::max(0, n - p); i < n; ++i) {
        window.append(m_lastData[i]);
    }

    for (int s = 0; s < steps; ++s) {
        double pred = 0.0;
        int wSize = window.size();
        for (int i = 0; i < p && i < wSize; ++i) {
            pred += m_coeffs[i] * window[wSize - 1 - i];
        }
        predictions.append(pred);
        window.append(pred);
    }

    m_stats.totalPredictions += steps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalFits > 0) ? m_timeSum / m_stats.totalFits : 0.0;

    emit predictionCompleted(steps);
    return predictions;
}

/** @brief 计算残差 @param data 原始数据 @return 残差序列 */
QVector<double> AutoRegressiveModel::residuals(const QVector<double>& data)
{
    QVector<double> resid;
    if (m_coeffs.isEmpty() || data.size() < static_cast<int>(m_coeffs.size()) + 1) {
        return resid;
    }

    int p = m_coeffs.size();
    resid.reserve(data.size() - p);

    for (int t = p; t < data.size(); ++t) {
        double predicted = 0.0;
        for (int i = 0; i < p; ++i) {
            predicted += m_coeffs[i] * data[t - 1 - i];
        }
        resid.append(data[t] - predicted);
    }
    return resid;
}

/** @brief 计算AIC @param data 数据 @param order 阶数 @return AIC值 */
double AutoRegressiveModel::computeAic(const QVector<double>& data, int order)
{
    int n = data.size();
    if (n < order + 1 || order < 1) return 1e30;

    QVector<double> acf = autocorrelation(data, order);
    if (acf.isEmpty() || acf[0] == 0.0) return 1e30;

    auto result = levinsonDurbin(acf, order);
    double sigma2 = result.second;

    if (sigma2 <= 0) return 1e30;
    double logLik = -0.5 * n * (1.0 + qLn(2.0 * M_PI) + qLn(sigma2));
    return -2.0 * logLik + 2.0 * order;
}

/** @brief 计算BIC @param data 数据 @param order 阶数 @return BIC值 */
double AutoRegressiveModel::computeBic(const QVector<double>& data, int order)
{
    int n = data.size();
    if (n < order + 1 || order < 1) return 1e30;

    QVector<double> acf = autocorrelation(data, order);
    if (acf.isEmpty() || acf[0] == 0.0) return 1e30;

    auto result = levinsonDurbin(acf, order);
    double sigma2 = result.second;

    if (sigma2 <= 0) return 1e30;
    double logLik = -0.5 * n * (1.0 + qLn(2.0 * M_PI) + qLn(sigma2));
    return -2.0 * logLik + static_cast<double>(order) * qLn(static_cast<double>(n));
}

/** @brief AR系数 @return 系数向量 */
QVector<double> AutoRegressiveModel::coefficients() const { return m_coeffs; }

/** @brief 白噪声方差 @return 方差 */
double AutoRegressiveModel::noiseVariance() const { return m_noiseVar; }

/** @brief 计算自相关函数 @param data 数据 @param maxLag 最大滞后 @return 自相关值 */
QVector<double> AutoRegressiveModel::autocorrelation(
    const QVector<double>& data, int maxLag)
{
    int n = data.size();
    if (n < 2) return {};

    /* 均值 */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += data[i];
    mean /= n;

    /* 方差 */
    double var = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = data[i] - mean;
        var += d * d;
    }
    if (var == 0.0) return {};

    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int t = 0; t < n - lag; ++t) {
            sum += (data[t] - mean) * (data[t + lag] - mean);
        }
        acf[lag] = sum / var;
    }
    return acf;
}

/** @brief Levinson-Durbin递归 @param acf 自相关 @param order 阶数 @return (系数,噪声方差) */
QPair<QVector<double>, double> AutoRegressiveModel::levinsonDurbin(
    const QVector<double>& acf, int order)
{
    QVector<double> a(order, 0.0);
    double sigma2 = acf[0]; /* 初始方差=自相关[0]=1.0(已归一化) */
    if (sigma2 <= 0) return {{}, 0.0};

    /* 用原始方差而非归一化后的 */
    /* 重新用未归一化的自协方差 */
    double r0 = acf[0];
    double err = r0;

    for (int k = 0; k < order; ++k) {
        /* 计算反射系数 */
        if (err == 0.0) break;
        double lambda = acf[k + 1];
        for (int j = 0; j < k; ++j) {
            lambda -= a[j] * acf[k - j];
        }
        lambda /= err;

        /* 更新系数 */
        QVector<double> newA = a;
        for (int j = 0; j < k; ++j) {
            newA[j] = a[j] - lambda * a[k - 1 - j];
        }
        newA[k] = lambda;
        a = newA;

        /* 更新误差 */
        err *= (1.0 - lambda * lambda);
        if (err <= 0) break;
    }

    return {a, err};
}

/** @brief 重置统计 */
void AutoRegressiveModel::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
