/**
 * @file FuzzyCMeans11.cpp
 * @brief FuzzyCMeans11 实现
 *
 * 实现模糊C均值聚类：Gustafson-Kessel距离与自适应簇体积约束矩阵估计。
 */

#include "utils/cluster233/FuzzyCMeans11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FuzzyCMeans11::FuzzyCMeans11(QObject *parent) : QObject(parent) {}
FuzzyCMeans11::~FuzzyCMeans11() = default;

/* ---- Configuration ---- */

void FuzzyCMeans11::setNumClusters(int c) { m_numClusters = qMax(2, c); }
void FuzzyCMeans11::setFuzziness(double m) { m_fuzziness = qMax(1.01, m); }
void FuzzyCMeans11::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void FuzzyCMeans11::setTolerance(double tol) { m_tolerance = qMax(1e-10, tol); }
void FuzzyCMeans11::setVolumeConstraint(double rho) { m_volumeConstraint = qMax(1e-6, rho); }

/* ---- Initialize membership matrix randomly ---- */

void FuzzyCMeans11::initMembership(int n)
{
    m_memberships.resize(n);
    for (int i = 0; i < n; ++i) {
        m_memberships[i].degrees.resize(m_numClusters);
        double sum = 0.0;
        for (int c = 0; c < m_numClusters; ++c) {
            m_memberships[i].degrees[c] = static_cast<double>(qrand()) / RAND_MAX + 0.1;
            sum += m_memberships[i].degrees[c];
        }
        // Normalize so each row sums to 1
        for (int c = 0; c < m_numClusters; ++c)
            m_memberships[i].degrees[c] /= sum;
    }
}

/* ---- Fuzzy covariance for cluster i ---- */

QVector<QVector<double>> FuzzyCMeans11::fuzzyCovariance(int ci) const
{
    int d = m_data[0].size();
    QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));

    // Weighted scatter matrix
    double um = 0.0;
    for (int i = 0; i < m_data.size(); ++i) {
        double w = qPow(m_memberships[i].degrees[ci], m_fuzziness);
        um += w;
        for (int r = 0; r < d; ++r) {
            double dr = m_data[i][r] - m_centroids[ci][r];
            for (int c = 0; c < d; ++c) {
                double dc = m_data[i][c] - m_centroids[ci][c];
                cov[r][c] += w * dr * dc;
            }
        }
    }

    // Normalize
    if (um > 1e-12) {
        for (int r = 0; r < d; ++r)
            for (int c = 0; c < d; ++c)
                cov[r][c] /= um;
    }

    // Regularize diagonal to avoid singularity
    for (int r = 0; r < d; ++r)
        cov[r][r] += 1e-8;

    return cov;
}

/* ---- Gustafson-Kessel distance ---- */

double FuzzyCMeans11::gkDistance(const QVector<double>& point, int ci) const
{
    int d = point.size();
    if (ci >= m_covariances.size()) return std::numeric_limits<double>::max();

    // Compute adaptive volume: det(A_i) = rho_i
    // Use det^(1/d) * identity scaling
    QVector<QVector<double>> covInv = invertMatrix(m_covariances[ci]);
    if (covInv.isEmpty()) return std::numeric_limits<double>::max();

    // Volume constraint: scale inverse by (det / rho)^(1/d)
    double detA = determinant(m_covariances[ci]);
    if (detA <= 0.0) detA = 1e-10;
    int dim = d;
    double scaleFactor = qPow(m_volumeConstraint / detA, 1.0 / dim);

    // d_GK^2 = (x - v)^T * [det(A_i)/rho_i]^{1/d} * A_i^{-1} * (x - v)
    QVector<double> diff(d);
    for (int i = 0; i < d; ++i)
        diff[i] = point[i] - m_centroids[ci][i];

    // Multiply: result = diff^T * (scaleFactor * covInv) * diff
    double dist2 = 0.0;
    for (int r = 0; r < d; ++r) {
        double rowSum = 0.0;
        for (int c = 0; c < d; ++c)
            rowSum += covInv[r][c] * diff[c];
        dist2 += diff[r] * rowSum * scaleFactor;
    }

    return qSqrt(qMax(0.0, dist2));
}

/* ---- Matrix determinant (d x d) ---- */

double FuzzyCMeans11::determinant(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return 0.0;
    if (n == 1) return mat[0][0];
    if (n == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

    // LU decomposition approach
    QVector<QVector<double>> lu(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            lu[i][j] = mat[i][j];

    double det = 1.0;
    for (int p = 0; p < n; ++p) {
        if (qAbs(lu[p][p]) < 1e-15) return 0.0;
        det *= lu[p][p];
        for (int i = p + 1; i < n; ++i) {
            double factor = lu[i][p] / lu[p][p];
            for (int j = p; j < n; ++j)
                lu[i][j] -= factor * lu[p][j];
        }
    }
    return det;
}

/* ---- Matrix inversion via Gauss-Jordan ---- */

QVector<QVector<double>> FuzzyCMeans11::invertMatrix(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    if (n == 0) return {};

    // Augmented matrix [A | I]
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }

    // Forward elimination
    for (int col = 0; col < n; ++col) {
        // Pivot
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > qAbs(aug[maxRow][col]))
                maxRow = row;
        }
        if (qAbs(aug[maxRow][col]) < 1e-15) return {};
        std::swap(aug[col], aug[maxRow]);

        double pivot = aug[col][col];
        for (int j = 0; j < 2 * n; ++j)
            aug[col][j] /= pivot;

        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    // Extract inverse
    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];
    return inv;
}

/* ---- Update centroids ---- */

void FuzzyCMeans11::updateCentroids()
{
    int d = m_data[0].size();
    int c = m_numClusters;
    int n = m_data.size();

    m_centroids.resize(c);
    for (int ci = 0; ci < c; ++ci) {
        m_centroids[ci].resize(d);
        m_centroids[ci].fill(0.0);
        double denom = 0.0;
        for (int i = 0; i < n; ++i) {
            double w = qPow(m_memberships[i].degrees[ci], m_fuzziness);
            denom += w;
            for (int j = 0; j < d; ++j)
                m_centroids[ci][j] += w * m_data[i][j];
        }
        if (denom > 1e-15)
            for (int j = 0; j < d; ++j)
                m_centroids[ci][j] /= denom;
    }
}

/* ---- Update membership matrix ---- */

void FuzzyCMeans11::updateMembership()
{
    int n = m_data.size();
    int c = m_numClusters;
    double exp = 1.0 / (m_fuzziness - 1.0);

    for (int i = 0; i < n; ++i) {
        // Compute GK distances to all clusters
        QVector<double> dists(c);
        for (int ci = 0; ci < c; ++ci)
            dists[ci] = gkDistance(m_data[i], ci);

        // Check for near-zero distance
        int zeroIdx = -1;
        for (int ci = 0; ci < c; ++ci) {
            if (dists[ci] < 1e-12) { zeroIdx = ci; break; }
        }

        if (zeroIdx >= 0) {
            m_memberships[i].degrees.fill(0.0);
            m_memberships[i].degrees[zeroIdx] = 1.0;
        } else {
            double sum = 0.0;
            for (int ci = 0; ci < c; ++ci) {
                double ratio = 1.0 / qPow(dists[ci], exp);
                m_memberships[i].degrees[ci] = ratio;
                sum += ratio;
            }
            for (int ci = 0; ci < c; ++ci)
                m_memberships[i].degrees[ci] /= sum;
        }

        // Update best cluster
        double best = -1.0;
        for (int ci = 0; ci < c; ++ci) {
            if (m_memberships[i].degrees[ci] > best) {
                best = m_memberships[i].degrees[ci];
                m_memberships[i].bestCluster = ci;
                m_memberships[i].bestMembership = best;
            }
        }
    }
}

/* ---- Compute objective J_m ---- */

double FuzzyCMeans11::computeObjective() const
{
    double J = 0.0;
    int n = m_data.size();
    int c = m_numClusters;
    for (int i = 0; i < n; ++i) {
        double um = qPow(m_memberships[i].degrees[m_memberships[i].bestCluster], m_fuzziness);
        // Use GK distance squared for objective
        double d2 = 0.0;
        int dim = m_data[i].size();
        for (int j = 0; j < dim; ++j) {
            double diff = m_data[i][j] - m_centroids[m_memberships[i].bestCluster][j];
            d2 += diff * diff;
        }
        J += um * d2;
    }
    return J;
}

/* ---- Fit ---- */

bool FuzzyCMeans11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_numClusters) return false;
    int d = data[0].size();

    m_data = data;
    initMembership(n);

    double prevObj = std::numeric_limits<double>::max();

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Step 1: Update centroids
        updateCentroids();

        // Step 2: Update covariance matrices
        m_covariances.resize(m_numClusters);
        for (int ci = 0; ci < m_numClusters; ++ci)
            m_covariances[ci] = fuzzyCovariance(ci);

        // Step 3: Update memberships with GK distance
        updateMembership();

        // Step 4: Check convergence
        double obj = computeObjective();
        if (qAbs(prevObj - obj) < m_tolerance) {
            m_stats.iterationsUsed = iter + 1;
            m_stats.finalObjective = obj;
            break;
        }
        prevObj = obj;
        m_stats.iterationsUsed = iter + 1;
        m_stats.finalObjective = obj;
        emit iterationCompleted(iter + 1, obj);
    }

    m_stats.numPoints = n;
    m_stats.numDimensions = d;
    m_stats.numClusters = m_numClusters;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_numClusters, m_stats.iterationsUsed, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

FuzzyCMeans11::Membership FuzzyCMeans11::predict(const QVector<double>& point) const
{
    Membership mem;
    if (m_centroids.isEmpty()) return mem;

    int c = m_numClusters;
    mem.degrees.resize(c);
    double exp = 1.0 / (m_fuzziness - 1.0);

    QVector<double> dists(c);
    for (int ci = 0; ci < c; ++ci)
        dists[ci] = gkDistance(point, ci);

    double sum = 0.0;
    for (int ci = 0; ci < c; ++ci) {
        double val = (dists[ci] < 1e-12) ? 1e12 : 1.0 / qPow(dists[ci], exp);
        mem.degrees[ci] = val;
        sum += val;
    }
    double best = -1.0;
    for (int ci = 0; ci < c; ++ci) {
        mem.degrees[ci] /= sum;
        if (mem.degrees[ci] > best) {
            best = mem.degrees[ci];
            mem.bestCluster = ci;
            mem.bestMembership = best;
        }
    }
    return mem;
}

/* ---- Accessors ---- */

QVector<QVector<double>> FuzzyCMeans11::centroids() const { return m_centroids; }
QVector<FuzzyCMeans11::Membership> FuzzyCMeans11::memberships() const { return m_memberships; }
QVector<QVector<QVector<double>>> FuzzyCMeans11::covariances() const { return m_covariances; }

/* ---- Reset ---- */

void FuzzyCMeans11::resetStatistics()
{
    m_data.clear();
    m_centroids.clear();
    m_memberships.clear();
    m_covariances.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
