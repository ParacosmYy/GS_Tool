/**
 * @file SignalClassifier4.cpp
 * @brief SignalClassifier4 实现
 *
 * 实现信号分类器：倒谱特征提取、GMM训练(EM算法)、BIC模型选择。
 */

#include "utils/signal206/SignalClassifier4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

SignalClassifier4::SignalClassifier4(QObject *parent) : QObject(parent) {}
SignalClassifier4::~SignalClassifier4() = default;

/* ---- Configuration ---- */

void SignalClassifier4::setNumClasses(int k) { m_numClasses = qMax(1, k); }
void SignalClassifier4::setNumMixtures(int m) { m_numMixtures = qMax(1, m); }
void SignalClassifier4::setFeatureDim(int dim) { m_featureDim = qMax(1, dim); }

/* ---- Diagonal covariance helper ---- */

QVector<QVector<double>> SignalClassifier4::diagCov(const QVector<double>& diag)
{
    int n = diag.size();
    QVector<QVector<double>> cov(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) cov[i][i] = qMax(diag[i], 1e-6);
    return cov;
}

/* ---- Log determinant ---- */

double SignalClassifier4::logDet(const QVector<QVector<double>>& cov)
{
    double ld = 0.0;
    for (int i = 0; i < cov.size(); ++i)
        ld += qLn(qMax(cov[i][i], 1e-10));
    return ld;
}

/* ---- Gaussian log-pdf ---- */

double SignalClassifier4::gaussianLogPdf(const QVector<double>& x, const QVector<double>& mean,
                                            const QVector<QVector<double>>& cov) const
{
    int d = x.size();
    double ld = logDet(cov);
    double mahal = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - mean[i];
        mahal += diff * diff / qMax(cov[i][i], 1e-10);
    }
    return -0.5 * (d * qLn(2.0 * M_PI) + ld + mahal);
}

/* ---- Extract cepstral features ---- */

QVector<double> SignalClassifier4::extractCepstral(const QVector<double>& frame) const
{
    int n = frame.size();
    // Compute log energy
    double energy = 0.0;
    for (double s : frame) energy += s * s;
    energy = qLn(qMax(energy, 1e-10));

    // Compute autocorrelation-based cepstral coefficients
    int order = m_featureDim - 1;
    QVector<double> autocorr(order + 1, 0.0);
    for (int k = 0; k <= order; ++k)
        for (int i = 0; i < n - k; ++i)
            autocorr[k] += frame[i] * frame[i + k];

    // Levinson-Durbin recursion for LPC -> cepstral
    QVector<double> lpc(order, 0.0);
    QVector<double> err(order + 1, 0.0);
    err[0] = qMax(autocorr[0], 1e-10);

    for (int m = 0; m < order; ++m) {
        double lambda = 0.0;
        for (int j = 0; j < m; ++j) lambda += lpc[j] * autocorr[m - j];
        lambda = (autocorr[m + 1] - lambda) / err[m];
        lpc[m] = lambda;
        for (int j = 0; j < m / 2; ++j) {
            double old = lpc[j];
            lpc[j] -= lambda * lpc[m - 1 - j];
            lpc[m - 1 - j] -= lambda * old;
        }
        err[m + 1] = err[m] * (1.0 - lambda * lambda);
    }

    // Convert LPC to cepstral coefficients
    QVector<double> cepstral(m_featureDim);
    cepstral[0] = energy;
    for (int i = 0; i < order; ++i) {
        double sum = 0.0;
        for (int k = 1; k <= i; ++k)
            sum += static_cast<double>(k) / (i + 1) * cepstral[k] * lpc[i + 1 - k - 1];
        cepstral[i + 1] = lpc[i] + sum;
    }
    return cepstral;
}

/* ---- Init GMM ---- */

QVector<SignalClassifier4::GMMComponent> SignalClassifier4::initGMM(
    const QVector<QVector<double>>& data, int m) const
{
    int n = data.size();
    int d = data[0].size();
    QVector<GMMComponent> components(m);

    // K-means++ initialization for means
    int first = QRandomGenerator::global()->bounded(n);
    components[0].mean = data[first];
    components[0].weight = 1.0 / m;

    for (int c = 1; c < m; ++c) {
        QVector<double> minDist(n, 1e18);
        for (int i = 0; i < n; ++i) {
            for (int cc = 0; cc < c; ++cc) {
                double d2 = 0.0;
                for (int j = 0; j < d; ++j) d2 += (data[i][j] - components[cc].mean[j]) * (data[i][j] - components[cc].mean[j]);
                minDist[i] = qMin(minDist[i], d2);
            }
        }
        double totalDist = 0.0;
        for (double d2 : minDist) totalDist += d2;
        double r = QRandomGenerator::global()->generateDouble() * totalDist;
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) { cumSum += minDist[i]; if (cumSum >= r) { chosen = i; break; } }
        components[c].mean = data[chosen];
        components[c].weight = 1.0 / m;
    }

    // Initialize with diagonal covariance from data variance
    QVector<double> var(d, 1.0);
    for (int j = 0; j < d; ++j) {
        double mean = 0.0;
        for (int i = 0; i < n; ++i) mean += data[i][j];
        mean /= n;
        double v = 0.0;
        for (int i = 0; i < n; ++i) v += (data[i][j] - mean) * (data[i][j] - mean);
        var[j] = v / n;
    }
    for (int c = 0; c < m; ++c) components[c].covariance = diagCov(var);
    return components;
}

/* ---- Fit GMM via EM ---- */

QVector<SignalClassifier4::GMMComponent> SignalClassifier4::fitGMM(
    const QVector<QVector<double>>& data, int numMixtures, int maxIter) const
{
    int n = data.size();
    if (n == 0) return {};
    int d = data[0].size();

    QVector<GMMComponent> components = initGMM(data, numMixtures);

    for (int iter = 0; iter < maxIter; ++iter) {
        // E-step: compute responsibilities
        QVector<QVector<double>> resp(n, QVector<double>(numMixtures, 0.0));
        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int c = 0; c < numMixtures; ++c) {
                resp[i][c] = components[c].weight * qExp(gaussianLogPdf(data[i], components[c].mean, components[c].covariance));
                total += resp[i][c];
            }
            if (total > 1e-30)
                for (int c = 0; c < numMixtures; ++c) resp[i][c] /= total;
        }

        // M-step: update parameters
        for (int c = 0; c < numMixtures; ++c) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) nk += resp[i][c];
            if (nk < 1e-10) continue;

            components[c].weight = nk / n;

            // Update mean
            for (int j = 0; j < d; ++j) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) sum += resp[i][c] * data[i][j];
                components[c].mean[j] = sum / nk;
            }

            // Update diagonal covariance
            QVector<double> var(d, 0.0);
            for (int j = 0; j < d; ++j) {
                for (int i = 0; i < n; ++i) {
                    double diff = data[i][j] - components[c].mean[j];
                    var[j] += resp[i][c] * diff * diff;
                }
                var[j] = qMax(var[j] / nk, 1e-6);
            }
            components[c].covariance = diagCov(var);
        }
    }
    return components;
}

/* ---- GMM log-likelihood ---- */

double SignalClassifier4::gmmLogLikelihood(const QVector<QVector<double>>& data,
                                              const QVector<GMMComponent>& gmm) const
{
    double ll = 0.0;
    for (const auto& x : data) {
        double p = 0.0;
        for (const auto& comp : gmm)
            p += comp.weight * qExp(gaussianLogPdf(x, comp.mean, comp.covariance));
        ll += qLn(qMax(p, 1e-30));
    }
    return ll;
}

/* ---- Compute BIC ---- */

double SignalClassifier4::computeBIC(const QVector<QVector<double>>& data,
                                       const QVector<GMMComponent>& gmm) const
{
    int n = data.size();
    int d = (n > 0) ? data[0].size() : 0;
    double ll = gmmLogLikelihood(data, gmm);
    // Number of parameters: mean(d) + diag_cov(d) + weight(1) per component - 1 (sum-to-one)
    int numParams = gmm.size() * (2 * d + 1) - 1;
    return -2.0 * ll + numParams * qLn(n);
}

/* ---- Train ---- */

void SignalClassifier4::train(const QVector<QVector<double>>& features, const QVector<int>& labels)
{
    QElapsedTimer timer;
    timer.start();

    m_classModels.resize(m_numClasses);

    for (int c = 0; c < m_numClasses; ++c) {
        QVector<QVector<double>> classData;
        for (int i = 0; i < features.size(); ++i)
            if (labels[i] == c) classData.append(features[i]);

        if (!classData.isEmpty())
            m_classModels[c] = fitGMM(classData, m_numMixtures);
    }

    m_stats.totalClassifications++;
    m_stats.numClasses = m_numClasses;
    m_stats.featureDim = m_featureDim;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClassifications;
}

/* ---- Classify ---- */

int SignalClassifier4::classify(const QVector<double>& features) const
{
    QElapsedTimer timer;
    timer.start();

    int bestClass = 0;
    double bestScore = -1e18;

    for (int c = 0; c < m_classModels.size(); ++c) {
        double score = 0.0;
        for (const auto& comp : m_classModels[c])
            score += comp.weight * qExp(gaussianLogPdf(features, comp.mean, comp.covariance));
        score = qLn(qMax(score, 1e-30));
        if (score > bestScore) { bestScore = score; bestClass = c; }
    }

    const_cast<SignalClassifier4*>(this)->m_timeSum += timer.elapsed();
    const_cast<SignalClassifier4*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalClassifications, 1ULL);

    emit classificationCompleted(bestClass, bestScore, timer.elapsed());
    return bestClass;
}

/* ---- Reset ---- */

void SignalClassifier4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_classModels.clear();
}
