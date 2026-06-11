/**
 * @file GaussianMixture36.cpp
 * @brief GaussianMixture36 实现
 *
 * 实现高斯混合模型：序贯蒙特卡洛EM与Rao-Blackwellized粒子采样实现非高斯分量混合建模。
 */

#include "utils/cluster296/GaussianMixture36.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussianMixture36::GaussianMixture36(QObject *parent)
    : QObject(parent) {}

GaussianMixture36::~GaussianMixture36() = default;

/* ---- Configuration ---- */

void GaussianMixture36::setNumComponents(int k) { m_k = qBound(1, k, 100); }
void GaussianMixture36::setNumParticles(int n) { m_numParticles = qBound(4, n, 1024); }
void GaussianMixture36::setMaxEMIterations(int maxIter) { m_maxIter = qBound(1, maxIter, 10000); }
void GaussianMixture36::setConvergenceTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }

/* ---- Gaussian PDF evaluation ---- */

double GaussianMixture36::gaussianPdf(double x, double mean, double var) const
{
    if (var <= 0.0) return 0.0;
    double diff = x - mean;
    return qExp(-0.5 * diff * diff / var) / (qSqrt(2.0 * M_PI * var));
}

/* ---- Initialize particles via k-means++ seeding ---- */

QVector<GaussianMixture36::Particle> GaussianMixture36::initializeParticles(
    const QVector<double>& data) const
{
    int n = data.size();
    QVector<Particle> particles(m_numParticles);

    for (int p = 0; p < m_numParticles; ++p) {
        particles[p].components.resize(m_k);
        particles[p].logWeight = 0.0;

        // K-means++ seeding: pick first center randomly
        QVector<double> centers;
        centers.reserve(m_k);
        centers.append(data[p % n]);

        for (int c = 1; c < m_k; ++c) {
            // Compute distance-based probabilities
            double totalDist = 0.0;
            QVector<double> dists;
            dists.reserve(n);
            for (int i = 0; i < n; ++i) {
                double minD = 1e300;
                for (double ct : centers)
                    minD = qMin(minD, qAbs(data[i] - ct));
                dists.append(minD * minD);
                totalDist += dists[i];
            }
            // Weighted random selection
            double threshold = (totalDist > 0.0) ? (totalDist * ((p * 137 + c * 31) % 1000) / 1000.0) : 0.0;
            double cumSum = 0.0;
            int chosen = 0;
            for (int i = 0; i < n; ++i) {
                cumSum += dists[i];
                if (cumSum >= threshold) { chosen = i; break; }
            }
            centers.append(data[chosen]);
        }

        for (int c = 0; c < m_k; ++c) {
            particles[p].components[c].weight = 1.0 / m_k;
            particles[p].components[c].mean = centers[c];
            // Estimate initial variance from data range
            double range = 1.0;
            if (n > 1) {
                double dmin = data[0], dmax = data[0];
                for (int i = 1; i < n; ++i) { dmin = qMin(dmin, data[i]); dmax = qMax(dmax, data[i]); }
                range = qMax(1e-6, dmax - dmin);
            }
            particles[p].components[c].variance = range * range / (m_k * m_k * 4.0);
            particles[p].components[c].variance = qMax(1e-6, particles[p].components[c].variance);
        }
    }
    return particles;
}

/* ---- E-step: compute responsibilities ---- */

void GaussianMixture36::eStep(Particle& p, const QVector<double>& data) const
{
    int n = data.size();
    for (int c = 0; c < m_k; ++c)
        p.components[c].sufficientStats.fill(0.0, 4); // N_k, sum_x, sum_x2, log_resp_sum
    for (int c = 0; c < m_k; ++c)
        p.components[c].sufficientStats.resize(4);

    for (int i = 0; i < n; ++i) {
        double denom = 0.0;
        QVector<double> resp(m_k);
        for (int c = 0; c < m_k; ++c) {
            resp[c] = p.components[c].weight * gaussianPdf(data[i], p.components[c].mean, p.components[c].variance);
            denom += resp[c];
        }
        if (denom <= 0.0) denom = 1e-300;
        for (int c = 0; c < m_k; ++c) {
            double r = resp[c] / denom;
            p.components[c].sufficientStats[0] += r;
            p.components[c].sufficientStats[1] += r * data[i];
            p.components[c].sufficientStats[2] += r * data[i] * data[i];
        }
    }
}

/* ---- M-step: update component parameters ---- */

void GaussianMixture36::mStep(Particle& p, const QVector<double>& data)
{
    int n = data.size();
    double totalN = 0.0;
    for (int c = 0; c < m_k; ++c)
        totalN += p.components[c].sufficientStats[0];

    for (int c = 0; c < m_k; ++c) {
        double Nk = p.components[c].sufficientStats[0];
        if (Nk < 1e-10) continue;
        p.components[c].weight = Nk / totalN;
        p.components[c].mean = p.components[c].sufficientStats[1] / Nk;
        double mean2 = p.components[c].mean * p.components[c].mean;
        p.components[c].variance = qMax(1e-6, p.components[c].sufficientStats[2] / Nk - mean2);
    }
}

/* ---- Normalize particle weights ---- */

void GaussianMixture36::normalizeWeights(QVector<Particle>& particles) const
{
    double maxLog = -1e300;
    for (auto& p : particles)
        maxLog = qMax(maxLog, p.logWeight);

    double sum = 0.0;
    for (auto& p : particles) {
        p.normalizedWeight = qExp(p.logWeight - maxLog);
        sum += p.normalizedWeight;
    }
    if (sum > 0.0)
        for (auto& p : particles)
            p.normalizedWeight /= sum;
}

/* ---- Systematic resampling ---- */

QVector<GaussianMixture36::Particle> GaussianMixture36::systematicResample(
    const QVector<Particle>& particles) const
{
    int N = particles.size();
    QVector<Particle> resampled(N);
    QVector<double> cumW(N);
    cumW[0] = particles[0].normalizedWeight;
    for (int i = 1; i < N; ++i)
        cumW[i] = cumW[i - 1] + particles[i].normalizedWeight;

    double step = 1.0 / N;
    double u0 = step * 0.5; // deterministic offset
    int j = 0;
    for (int i = 0; i < N; ++i) {
        double u = u0 + i * step;
        while (j < N - 1 && cumW[j] < u) j++;
        resampled[i] = particles[j];
        resampled[i].logWeight = 0.0;
    }
    return resampled;
}

/* ---- Rao-Blackwellized move: perturb component means/variances ---- */

void GaussianMixture36::raoBlackwellizedMove(Particle& p, const QVector<double>& data)
{
    double dataVar = 1.0;
    int n = data.size();
    if (n > 1) {
        double mean = 0.0;
        for (double x : data) mean += x;
        mean /= n;
        for (double x : data) dataVar += (x - mean) * (x - mean);
        dataVar /= n;
    }
    double perturbScale = qMax(1e-6, qSqrt(dataVar) * 0.01);

    for (int c = 0; c < m_k; ++c) {
        // Small deterministic perturbation based on index
        double delta = perturbScale * qSin(c * 2.39996 + dataVar);
        p.components[c].mean += delta;
        p.components[c].variance = qMax(1e-6, p.components[c].variance * (1.0 + 0.005 * qCos(c * 1.7)));
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture36::logLikelihood(const QVector<Component>& comps,
                                         const QVector<double>& data) const
{
    double ll = 0.0;
    for (double x : data) {
        double p = 0.0;
        for (auto& c : comps)
            p += c.weight * gaussianPdf(x, c.mean, c.variance);
        ll += qLn(qMax(1e-300, p));
    }
    return ll;
}

/* ---- Main fit ---- */

GaussianMixture36::EMResult GaussianMixture36::fit(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    EMResult result;
    int n = data.size();
    if (n < m_k) return result;

    // Initialize SMC particles
    auto particles = initializeParticles(data);

    double prevLL = -1e300;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // For each particle: E-step, M-step, compute weight
        for (auto& p : particles) {
            eStep(p, data);
            mStep(p, data);
            raoBlackwellizedMove(p, data);
            p.logWeight = logLikelihood(p.components, data);
        }

        normalizeWeights(particles);

        // Compute weighted log-likelihood (ESS check)
        double ess = 0.0;
        for (auto& p : particles)
            ess += p.normalizedWeight * p.normalizedWeight;
        ess = 1.0 / qMax(1e-300, ess);

        // Resample if ESS drops below half
        if (ess < m_numParticles / 2.0)
            particles = systematicResample(particles);

        // Find best particle
        int best = 0;
        for (int i = 1; i < particles.size(); ++i)
            if (particles[i].logWeight > particles[best].logWeight) best = i;

        double curLL = particles[best].logWeight;

        if (qAbs(curLL - prevLL) < m_tol) {
            result.converged = true;
            result.iterations = iter + 1;
            result.logLikelihood = curLL;
            result.components = particles[best].components;
            break;
        }
        prevLL = curLL;

        if (iter == m_maxIter - 1) {
            result.logLikelihood = curLL;
            result.iterations = m_maxIter;
            result.components = particles[best].components;
        }
    }

    m_components = result.components;
    m_stats.numComponents = m_k;
    m_stats.numParticles = m_numParticles;
    m_stats.totalFits++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitDone(m_k, result.iterations, result.logLikelihood, elapsed);
    return result;
}

/* ---- Posterior probabilities ---- */

QVector<QVector<double>> GaussianMixture36::posteriorProbabilities(
    const QVector<double>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));

    for (int i = 0; i < n; ++i) {
        double denom = 0.0;
        for (int c = 0; c < m_k; ++c) {
            resp[i][c] = m_components[c].weight *
                gaussianPdf(data[i], m_components[c].mean, m_components[c].variance);
            denom += resp[i][c];
        }
        if (denom > 0.0)
            for (int c = 0; c < m_k; ++c)
                resp[i][c] /= denom;
    }
    return resp;
}

/* ---- Sample from mixture ---- */

QVector<double> GaussianMixture36::sample(int count) const
{
    QVector<double> samples;
    samples.reserve(count);

    // Deterministic quasi-random sampling via stratified approach
    for (int i = 0; i < count; ++i) {
        double u = (i + 0.5) / count;
        double cumW = 0.0;
        int c = 0;
        for (; c < m_k; ++c) {
            cumW += m_components[c].weight;
            if (u <= cumW) break;
        }
        c = qMin(c, m_k - 1);

        // Inverse CDF of Gaussian via Box-Muller (deterministic variant)
        double u1 = qMax(1e-10, (i * 0.6180339887) - qFloor(i * 0.6180339887));
        double u2 = (i * 0.3183098861) - qFloor(i * 0.3183098861);
        double z = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
        samples.append(m_components[c].mean + z * qSqrt(m_components[c].variance));
    }
    return samples;
}

/* ---- Reset ---- */

void GaussianMixture36::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_components.clear();
}
