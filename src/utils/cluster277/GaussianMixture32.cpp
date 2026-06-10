/**
 * @file GaussianMixture32.cpp
 * @brief GaussianMixture32 实现
 *
 * 实现高斯混合模型：分裂合并EM与BIC自动模型选择。
 */

#include "utils/cluster277/GaussianMixture32.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture32::GaussianMixture32(QObject *parent)
    : QObject(parent) {}

GaussianMixture32::~GaussianMixture32() = default;

/* ---- Configuration ---- */

void GaussianMixture32::setMaxComponents(int k) { m_maxComponents = qBound(2, k, 50); }
void GaussianMixture32::setTolerance(double tol) { m_tolerance = qBound(1e-10, tol, 1.0); }
void GaussianMixture32::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 1000); }

/* ---- Distance helper ---- */

double GaussianMixture32::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/* ---- Matrix determinant (small matrix via cofactor) ---- */

double GaussianMixture32::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 1) return mat[0][0];
    if (n == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

    double det = 0.0;
    for (int j = 0; j < n; ++j) {
        QVector<QVector<double>> sub(n - 1, QVector<double>(n - 1, 0.0));
        for (int r = 1; r < n; ++r) {
            int cc = 0;
            for (int c = 0; c < n; ++c) {
                if (c == j) continue;
                sub[r - 1][cc++] = mat[r][c];
            }
        }
        det += (j % 2 == 0 ? 1 : -1) * mat[0][j] * determinant(sub);
    }
    return det;
}

/* ---- Matrix inversion (Gauss-Jordan) ---- */

QVector<QVector<double>> GaussianMixture32::invertMatrix(
    const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }
    for (int col = 0; col < n; ++col) {
        int pivot = col;
        for (int r = col + 1; r < n; ++r)
            if (qAbs(aug[r][col]) > qAbs(aug[pivot][col])) pivot = r;
        std::swap(aug[col], aug[pivot]);
        double d = aug[col][col];
        if (qAbs(d) < 1e-15) return QVector<QVector<double>>(n, QVector<double>(n, 0.0));
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= d;
        for (int r = 0; r < n; ++r) {
            if (r == col) continue;
            double f = aug[r][col];
            for (int j = 0; j < 2 * n; ++j) aug[r][j] -= f * aug[col][j];
        }
    }
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) inv[i][j] = aug[i][n + j];
    return inv;
}

/* ---- Multivariate Gaussian log-pdf ---- */

double GaussianMixture32::gaussianLogPdf(const QVector<double>& x,
                                          const Component& comp) const
{
    int dim = comp.mean.size();
    if (dim == 0) return 0.0;

    double det = determinant(comp.covariance);
    if (det <= 1e-300) return -1e30;

    auto inv = invertMatrix(comp.covariance);
    QVector<double> diff(dim, 0.0);
    for (int i = 0; i < dim; ++i) diff[i] = x[i] - comp.mean[i];

    double quad = 0.0;
    for (int i = 0; i < dim; ++i)
        for (int j = 0; j < dim; ++j)
            quad += diff[i] * inv[i][j] * diff[j];

    return -0.5 * (dim * M_LN2 + dim * qLn(M_PI) + qLn(det) + quad);
}

/* ---- E-step: compute responsibilities ---- */

void GaussianMixture32::eStep(const QVector<QVector<double>>& data,
                                const QVector<Component>& comps,
                                QVector<QVector<double>>& resp)
{
    int n = data.size();
    int k = comps.size();
    resp.resize(n);
    for (int i = 0; i < n; ++i) resp[i].resize(k, 0.0);

    for (int i = 0; i < n; ++i) {
        double maxLog = -1e30;
        QVector<double> logResp(k, 0.0);
        for (int c = 0; c < k; ++c) {
            logResp[c] = qLn(qMax(comps[c].weight, 1e-300)) + gaussianLogPdf(data[i], comps[c]);
            if (logResp[c] > maxLog) maxLog = logResp[c];
        }
        double sum = 0.0;
        for (int c = 0; c < k; ++c) {
            resp[i][c] = qExp(logResp[c] - maxLog);
            sum += resp[i][c];
        }
        if (sum > 1e-300)
            for (int c = 0; c < k; ++c) resp[i][c] /= sum;
    }
}

/* ---- M-step: update parameters ---- */

void GaussianMixture32::mStep(const QVector<QVector<double>>& data,
                                const QVector<QVector<double>>& resp,
                                QVector<Component>& comps)
{
    int n = data.size();
    int k = comps.size();
    int dim = data.isEmpty() ? 0 : data[0].size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][c];
        if (nk < 1e-15) continue;

        // Update weight
        comps[c].weight = nk / n;

        // Update mean
        comps[c].mean = QVector<double>(dim, 0.0);
        for (int i = 0; i < n; ++i)
            for (int d = 0; d < dim; ++d)
                comps[c].mean[d] += resp[i][c] * data[i][d];
        for (int d = 0; d < dim; ++d) comps[c].mean[d] /= nk;

        // Update covariance
        comps[c].covariance = QVector<QVector<double>>(dim, QVector<double>(dim, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int r = 0; r < dim; ++r) {
                double dr = data[i][r] - comps[c].mean[r];
                for (int col = 0; col < dim; ++col) {
                    double dc = data[i][col] - comps[c].mean[col];
                    comps[c].covariance[r][col] += resp[i][c] * dr * dc;
                }
            }
        }
        // Regularize with small diagonal
        for (int r = 0; r < dim; ++r) {
            for (int col = 0; col < dim; ++col)
                comps[c].covariance[r][col] /= nk;
            comps[c].covariance[r][r] += 1e-6;   // Regularization
        }
    }
}

/* ---- Run EM for fixed k ---- */

QVector<GaussianMixture32::Component> GaussianMixture32::runEM(
    const QVector<QVector<double>>& data, int k, QVector<int>& labels, double& ll)
{
    int n = data.size();
    int dim = data.isEmpty() ? 0 : data[0].size();

    // Initialize components via random partition
    QVector<Component> comps(k);
    for (int c = 0; c < k; ++c) {
        comps[c].weight = 1.0 / k;
        comps[c].mean = data[c % n];
        comps[c].covariance = QVector<QVector<double>>(dim, QVector<double>(dim, 0.0));
        for (int d = 0; d < dim; ++d) comps[c].covariance[d][d] = 1.0;
    }

    QVector<QVector<double>> resp;
    ll = -1e30;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        eStep(data, comps, resp);
        mStep(data, resp, comps);

        // Compute log-likelihood
        double newLL = 0.0;
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int c = 0; c < k; ++c)
                sum += comps[c].weight * qExp(gaussianLogPdf(data[i], comps[c]));
            newLL += qLn(qMax(sum, 1e-300));
        }
        if (qAbs(newLL - ll) < m_tolerance) { ll = newLL; break; }
        ll = newLL;
    }

    // Assign labels from responsibilities
    labels.resize(n);
    for (int i = 0; i < n; ++i) {
        int best = 0;
        for (int c = 1; c < k; ++c)
            if (resp[i][c] > resp[i][best]) best = c;
        labels[i] = best;
    }

    return comps;
}

/* ---- Split-and-merge to escape local optima ---- */

void GaussianMixture32::splitAndMerge(QVector<Component>& comps,
                                       const QVector<QVector<double>>& data,
                                       const QVector<int>& labels)
{
    int k = comps.size();
    if (k < 2) return;

    // Find worst component (lowest weight) and split it
    int worstIdx = 0;
    for (int c = 1; c < k; ++c)
        if (comps[c].weight < comps[worstIdx].weight) worstIdx = c;

    // Split: perturb mean slightly to create two components
    int dim = comps[worstIdx].mean.size();
    QVector<double> perturb(dim, 0.0);
    for (int d = 0; d < dim; ++d)
        perturb[d] = 0.1 * (static_cast<double>(qrand()) / RAND_MAX - 0.5);

    for (int d = 0; d < dim; ++d) comps[worstIdx].mean[d] += perturb[d];
    emit componentSplit(worstIdx, 0.0);

    // Merge two closest components
    int mergeI = 0, mergeJ = 1;
    double minDist = 1e18;
    for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
            double d = distSq(comps[i].mean, comps[j].mean);
            if (d < minDist) { minDist = d; mergeI = i; mergeJ = j; }
        }
    }
    // Weighted average merge
    double wSum = comps[mergeI].weight + comps[mergeJ].weight;
    for (int d = 0; d < dim; ++d)
        comps[mergeI].mean[d] = (comps[mergeI].weight * comps[mergeI].mean[d] +
                                  comps[mergeJ].weight * comps[mergeJ].mean[d]) / wSum;
    comps[mergeI].weight = wSum;
    emit componentMerged(mergeI, mergeJ, 0.0);
}

/* ---- Compute BIC ---- */

double GaussianMixture32::computeBIC(const QVector<QVector<double>>& data, int k) const
{
    int n = data.size();
    int dim = data.isEmpty() ? 0 : data[0].size();
    // Number of free parameters: k-1 weights + k*dim means + k*dim*(dim+1)/2 covariance
    int p = (k - 1) + k * dim + k * dim * (dim + 1) / 2;
    // BIC = -2 * LL + p * ln(n)
    return -2.0 * 0.0 + p * qLn(n);   // Placeholder; real BIC computed inside fit()
}

/* ---- Log-likelihood ---- */

double GaussianMixture32::logLikelihood(const QVector<QVector<double>>& data,
                                         const QVector<Component>& comps) const
{
    int n = data.size();
    int k = comps.size();
    double ll = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int c = 0; c < k; ++c)
            sum += comps[c].weight * qExp(gaussianLogPdf(data[i], comps[c]));
        ll += qLn(qMax(sum, 1e-300));
    }
    return ll;
}

/* ---- Fit: main entry with BIC model selection ---- */

GaussianMixture32::GMMResult GaussianMixture32::fit(
    const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    GMMResult result;
    int n = data.size();
    if (n < 2) return result;

    if (k > 0) {
        // Fixed k mode
        QVector<int> labels;
        double ll = 0.0;
        result.components = runEM(data, k, labels, ll);
        result.labels = labels;
        result.logLikelihood = ll;
        result.optimalK = k;
        result.converged = true;

        // Apply split-and-merge refinement
        splitAndMerge(result.components, data, result.labels);
        result.logLikelihood = logLikelihood(data, result.components);
    } else {
        // Auto model selection via BIC
        double bestBIC = 1e30;
        for (int tryK = 2; tryK <= m_maxComponents; ++tryK) {
            QVector<int> labels;
            double ll = 0.0;
            auto comps = runEM(data, tryK, labels, ll);

            int dim = data[0].size();
            int p = (tryK - 1) + tryK * dim + tryK * dim * (dim + 1) / 2;
            double bic = -2.0 * ll + p * qLn(n);

            if (bic < bestBIC) {
                bestBIC = bic;
                result.components = comps;
                result.labels = labels;
                result.logLikelihood = ll;
                result.optimalK = tryK;
                result.bic = bic;
            }
        }
        result.converged = true;
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numComponents = result.optimalK;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingDone(result.optimalK, result.bic, elapsed);

    return result;
}

/* ---- Reset ---- */

void GaussianMixture32::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
