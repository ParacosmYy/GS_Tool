/**
 * @file GaussianMixture.cpp
 * @brief 高斯混合模型实现 — EM算法(一维)
 */

#include "utils/gmixture/GaussianMixture.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
GaussianMixture::GaussianMixture(QObject* parent)
    : QObject(parent)
{
}

/** @brief 拟合高斯混合模型 */
double GaussianMixture::fit(const QVector<double>& data, int k, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < k || k <= 0) return 0.0;

    int n = data.size();
    m_components.resize(k);

    /* 初始化: 均匀划分数据范围 */
    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());
    double range = maxVal - minVal;

    for (int j = 0; j < k; ++j) {
        m_components[j].mean = minVal + range * (j + 0.5) / k;
        m_components[j].variance = (range / k) * (range / k);
        m_components[j].weight = 1.0 / k;
    }

    QVector<QVector<double>> resp(k);
    double ll = 0.0;

    for (int iter = 0; iter < maxIter; ++iter) {
        eStep(data, resp);
        mStep(data, resp);
        ll = logLikelihood(data);

        /* 防止方差退化 */
        for (int j = 0; j < k; ++j) {
            if (m_components[j].variance < 1e-10)
                m_components[j].variance = 1e-10;
        }
    }

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(k, ll);
    return ll;
}

/** @brief 预测样本属于哪个分量 */
int GaussianMixture::predict(double sample) const
{
    m_stats.totalPredictions++; // mutable

    auto resp = responsibilities(sample);
    int bestIdx = 0;
    double bestProb = -1.0;

    for (int j = 0; j < resp.size(); ++j) {
        if (resp[j] > bestProb) {
            bestProb = resp[j];
            bestIdx = j;
        }
    }
    return bestIdx;
}

/** @brief 计算责任度(后验概率) */
QVector<double> GaussianMixture::responsibilities(double sample) const
{
    int k = m_components.size();
    if (k == 0) return {};

    QVector<double> resp(k);
    double total = 0.0;

    for (int j = 0; j < k; ++j) {
        resp[j] = m_components[j].weight * gaussianPdf(sample, m_components[j]);
        total += resp[j];
    }

    if (total > 1e-300) {
        for (int j = 0; j < k; ++j) resp[j] /= total;
    }
    return resp;
}

/** @brief 重置统计 */
void GaussianMixture::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算单个高斯PDF */
double GaussianMixture::gaussianPdf(double x, const Component& comp) const
{
    double diff = x - comp.mean;
    double exponent = -0.5 * diff * diff / comp.variance;
    double norm = 1.0 / std::sqrt(2.0 * M_PI * comp.variance);
    return norm * std::exp(exponent);
}

/** @brief E步 — 计算责任度 */
void GaussianMixture::eStep(const QVector<double>& data,
                             QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = m_components.size();

    for (int j = 0; j < k; ++j) resp[j].resize(n);

    for (int i = 0; i < n; ++i) {
        double total = 0.0;
        for (int j = 0; j < k; ++j) {
            resp[j][i] = m_components[j].weight
                         * gaussianPdf(data[i], m_components[j]);
            total += resp[j][i];
        }
        if (total > 1e-300) {
            for (int j = 0; j < k; ++j) resp[j][i] /= total;
        }
    }
}

/** @brief M步 — 更新参数 */
void GaussianMixture::mStep(const QVector<double>& data,
                             const QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = m_components.size();

    for (int j = 0; j < k; ++j) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[j][i];

        if (nk < 1e-300) continue;

        double meanSum = 0.0;
        for (int i = 0; i < n; ++i) meanSum += resp[j][i] * data[i];
        m_components[j].mean = meanSum / nk;

        double varSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double diff = data[i] - m_components[j].mean;
            varSum += resp[j][i] * diff * diff;
        }
        m_components[j].variance = varSum / nk;
        m_components[j].weight = nk / n;
    }
}

/** @brief 计算对数似然 */
double GaussianMixture::logLikelihood(const QVector<double>& data) const
{
    double ll = 0.0;
    int k = m_components.size();

    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < k; ++j) {
            sum += m_components[j].weight * gaussianPdf(data[i], m_components[j]);
        }
        if (sum > 1e-300) ll += std::log10(sum);
    }
    return ll;
}
