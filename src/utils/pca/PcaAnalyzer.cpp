/**
 * @file PcaAnalyzer.cpp
 * @brief PCA主成分分析器实现
 */

#include "utils/pca/PcaAnalyzer.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

PcaAnalyzer::PcaAnalyzer(QObject* parent)
    : QObject(parent), m_varianceThreshold(0.95), m_maxComponents(0), m_timeSum(0.0) {}

void PcaAnalyzer::setVarianceThreshold(double t) { m_varianceThreshold = qBound(0.0, t, 1.0); }
void PcaAnalyzer::setMaxComponents(int max) { m_maxComponents = qMax(0, max); }

PcaAnalyzer::PcaResult PcaAnalyzer::analyze(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    PcaResult result;
    if (data.isEmpty()) return result;

    int n = data.size();
    int d = data[0].size();

    /* 均值中心化 */
    QVector<double> mean(d, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j) mean[j] += data[i][j];
    for (int j = 0; j < d; ++j) mean[j] /= n;

    QVector<QVector<double>> centered(n, QVector<double>(d));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            centered[i][j] = data[i][j] - mean[j];

    /* 协方差矩阵 */
    QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            for (int k = j; k < d; ++k) {
                double v = centered[i][j] * centered[i][k] / (n - 1);
                cov[j][k] += v;
                if (j != k) cov[k][j] += v;
            }

    /* 幂迭代法求特征值/特征向量 */
    QVector<QPair<double, QVector<double>>> eigens;
    QVector<QVector<double>> deflated = cov;

    for (int iter = 0; iter < d; ++iter) {
        QVector<double> v(d, 1.0 / qSqrt(d));
        double eigenvalue = 0.0;

        for (int power = 0; power < 100; ++power) {
            QVector<double> Av(d, 0.0);
            for (int j = 0; j < d; ++j)
                for (int k = 0; k < d; ++k)
                    Av[j] += deflated[j][k] * v[k];

            double norm = 0.0;
            for (double val : Av) norm += val * val;
            norm = qSqrt(norm);
            if (norm < 1e-15) break;

            eigenvalue = 0.0;
            for (int j = 0; j < d; ++j) {
                v[j] = Av[j] / norm;
                eigenvalue += Av[j] * v[j];
            }
        }

        if (eigenvalue < 1e-10) break;
        eigens.append({eigenvalue, v});

        /* Hotelling压缩 */
        for (int j = 0; j < d; ++j)
            for (int k = 0; k < d; ++k)
                deflated[j][k] -= eigenvalue * v[j] * v[k];
    }

    /* 按特征值降序排列 */
    std::sort(eigens.begin(), eigens.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    double totalVariance = 0.0;
    for (const auto& e : eigens) totalVariance += e.first;

    result.eigenvalues.resize(eigens.size());
    result.explainedVarianceRatio.resize(eigens.size());
    result.components.resize(eigens.size());

    for (int i = 0; i < eigens.size(); ++i) {
        result.eigenvalues[i] = eigens[i].first;
        result.explainedVarianceRatio[i] = (totalVariance > 0) ? eigens[i].first / totalVariance : 0.0;
        result.components[i] = eigens[i].second;
    }

    /* 选择主成分数量 */
    double cumulative = 0.0;
    int selected = eigens.size();
    for (int i = 0; i < eigens.size(); ++i) {
        cumulative += result.explainedVarianceRatio[i];
        if (cumulative >= m_varianceThreshold) { selected = i + 1; break; }
    }
    if (m_maxComponents > 0) selected = qMin(selected, m_maxComponents);
    result.selectedComponents = selected;
    result.cumulativeVariance = cumulative;

    /* 投影数据 */
    result.projected.resize(n);
    for (int i = 0; i < n; ++i) {
        result.projected[i].resize(selected);
        for (int c = 0; c < selected; ++c) {
            double dot = 0.0;
            for (int j = 0; j < d; ++j) dot += centered[i][j] * result.components[c][j];
            result.projected[i][c] = dot;
        }
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalAnalyses;
    double compSum = m_stats.avgComponents * (m_stats.totalAnalyses - 1) + selected;
    m_stats.avgComponents = compSum / m_stats.totalAnalyses;
    double varSum = m_stats.avgCumulativeVariance * (m_stats.totalAnalyses - 1) + cumulative;
    m_stats.avgCumulativeVariance = varSum / m_stats.totalAnalyses;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisComplete(selected, cumulative);
    return result;
}

QVector<QVector<double>> PcaAnalyzer::covarianceMatrix(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return {};
    int n = data.size();
    int d = data[0].size();

    QVector<double> mean(d, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j) mean[j] += data[i][j];
    for (int j = 0; j < d; ++j) mean[j] /= n;

    QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            for (int k = j; k < d; ++k) {
                double v = (data[i][j] - mean[j]) * (data[i][k] - mean[k]) / (n - 1);
                cov[j][k] += v;
                if (j != k) cov[k][j] += v;
            }
    return cov;
}

void PcaAnalyzer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
