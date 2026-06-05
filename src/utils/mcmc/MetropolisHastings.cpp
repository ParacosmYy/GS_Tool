/**
 * @file MetropolisHastings.cpp
 * @brief Metropolis-Hastings采样器实现
 */

#include "MetropolisHastings.h"
#include <QElapsedTimer>
#include <cmath>
#include <cstdlib>
#include <algorithm>

MetropolisHastings::MetropolisHastings(QObject* parent)
    : QObject(parent)
    , m_proposalWidth(1.0)
    , m_accepted(0)
    , m_total(0)
    , m_timeSum(0.0)
{
}

void MetropolisHastings::setTargetDistribution(std::function<double(double)> logPdf)
{
    m_logPdf = logPdf;
}

void MetropolisHastings::setProposalWidth(double sigma)
{
    m_proposalWidth = qMax(1e-10, sigma);
}

QVector<double> MetropolisHastings::sample(double initial, int nSamples, int burnIn)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> chain;
    chain.reserve(nSamples);

    double current = initial;
    double currentLogP = m_logPdf ? m_logPdf(current) : -1e30;
    m_accepted = 0;
    m_total = 0;

    int totalIter = nSamples + burnIn;
    for (int i = 0; i < totalIter; ++i) {
        /* 高斯提议 */
        double u1 = static_cast<double>(std::rand()) / RAND_MAX;
        double u2 = static_cast<double>(std::rand()) / RAND_MAX;
        u1 = qMax(1e-15, u1);
        double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        double proposal = current + m_proposalWidth * z;

        double proposalLogP = m_logPdf ? m_logPdf(proposal) : -1e30;
        double logAlpha = proposalLogP - currentLogP;

        m_total++;
        if (std::log(static_cast<double>(std::rand()) / RAND_MAX) < logAlpha) {
            current = proposal;
            currentLogP = proposalLogP;
            m_accepted++;
        }

        if (i >= burnIn)
            chain.append(current);
    }

    m_stats.totalSamples += nSamples;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples;

    emit samplingCompleted(nSamples, acceptanceRate());
    return chain;
}

QVector<double> MetropolisHastings::autocorrelation(const QVector<double>& chain,
                                                       int maxLag)
{
    int n = chain.size();
    if (n < 2) return {};

    double mean = 0.0;
    for (double x : chain) mean += x;
    mean /= n;

    double var = 0.0;
    for (double x : chain) var += (x - mean) * (x - mean);
    if (var < 1e-15) return QVector<double>(maxLag, 0.0);

    QVector<double> acf;
    for (int lag = 0; lag <= qMin(maxLag, n - 1); ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i)
            sum += (chain[i] - mean) * (chain[i + lag] - mean);
        acf.append(sum / (var * n));
    }
    return acf;
}

int MetropolisHastings::effectiveSampleSize(const QVector<double>& chain)
{
    auto acf = autocorrelation(chain, qMin(100, chain.size() / 2));
    if (acf.size() < 2) return chain.size();

    double sum = -1.0;
    for (int i = 1; i < acf.size(); i += 2) {
        if (i + 1 < acf.size())
            sum += acf[i] + acf[i + 1];
        else
            sum += acf[i];
        if (sum < 0) break;
    }
    sum = qMax(0.5, 1.0 + 2.0 * sum);
    return qMax(1, static_cast<int>(chain.size() / sum));
}

double MetropolisHastings::acceptanceRate() const
{
    return (m_total > 0) ? static_cast<double>(m_accepted) / m_total : 0.0;
}

MetropolisHastings::Stats MetropolisHastings::stats() const { return m_stats; }

void MetropolisHastings::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
