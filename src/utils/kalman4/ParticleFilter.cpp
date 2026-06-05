/**
 * @file ParticleFilter.cpp
 * @brief 粒子滤波器实现 — 序贯蒙特卡洛非线性状态估计
 */

#include "utils/kalman4/ParticleFilter.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
ParticleFilter::ParticleFilter(QObject* parent)
    : QObject(parent)
    , m_stateDim(1)
    , m_obsDim(1)
    , m_processNoise(0.1)
    , m_timeSum(0.0)
{
}

/** @brief 初始化粒子滤波器
 *  @param stateDim 状态维度
 *  @param obsDim 观测维度 */
void ParticleFilter::initialize(int stateDim, int obsDim)
{
    m_stateDim = qMax(1, stateDim);
    m_obsDim = qMax(1, obsDim);

    const int numParticles = 100;
    m_particles.resize(numParticles);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(0.0, 1.0);

    double uniformWeight = 1.0 / static_cast<double>(numParticles);
    for (auto& p : m_particles) {
        p.state.resize(m_stateDim);
        for (int d = 0; d < m_stateDim; ++d) {
            p.state[d] = dist(gen) * 0.1;
        }
        p.weight = uniformWeight;
    }
}

/** @brief 预测步骤(状态转移) */
void ParticleFilter::predict()
{
    QElapsedTimer timer;
    timer.start();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> noise(0.0, m_processNoise);

    for (auto& p : m_particles) {
        if (m_stateFunc) {
            p.state = m_stateFunc(p.state);
        }
        for (int d = 0; d < m_stateDim; ++d) {
            p.state[d] += noise(gen);
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalPredictions;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalPredictions + m_stats.totalUpdates);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit predictionCompleted(m_particles.size());
}

/** @brief 更新步骤(观测修正)
 *  @param measurement 观测值 */
void ParticleFilter::update(double measurement)
{
    QElapsedTimer timer;
    timer.start();

    /* 更新权重: 使用观测似然函数 */
    for (auto& p : m_particles) {
        double likelihood = 1.0;
        if (m_obsFunc) {
            likelihood = m_obsFunc(p.state, measurement);
        } else {
            /* 默认: 基于第一个状态分量与观测值的高斯似然 */
            double diff = (p.state.isEmpty() ? 0.0 : p.state[0]) - measurement;
            likelihood = qExp(-0.5 * diff * diff / (m_processNoise * m_processNoise));
        }
        p.weight *= qMax(1e-300, likelihood);
    }

    normalizeWeights();

    /* 计算ESS并在需要时重采样 */
    double ess = effectiveSampleSize();
    if (ess < static_cast<double>(m_particles.size()) * 0.5) {
        resample();
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    double total = static_cast<double>(m_stats.totalPredictions + m_stats.totalUpdates);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit updateCompleted(ess);
}

/** @brief 获取当前加权平均状态 @return 状态向量 */
QVector<double> ParticleFilter::state() const
{
    QVector<double> est(m_stateDim, 0.0);
    for (const auto& p : m_particles) {
        for (int d = 0; d < m_stateDim; ++d) {
            est[d] += p.weight * p.state[d];
        }
    }
    return est;
}

/** @brief 设置状态转移函数 @param func 转移函数 */
void ParticleFilter::setStateFunc(const StateFunc& func)
{
    m_stateFunc = func;
}

/** @brief 设置观测似然函数 @param func 似然函数 */
void ParticleFilter::setObsFunc(const ObsFunc& func)
{
    m_obsFunc = func;
}

/** @brief 设置过程噪声标准差 @param sigma 噪声标准差 */
void ParticleFilter::setProcessNoise(double sigma)
{
    m_processNoise = qMax(1e-10, sigma);
}

/** @brief 获取有效粒子数 @return ESS */
double ParticleFilter::effectiveSampleSize() const
{
    double sumSq = 0.0;
    for (const auto& p : m_particles) {
        sumSq += p.weight * p.weight;
    }
    return (sumSq > 1e-300) ? 1.0 / sumSq : 0.0;
}

/** @brief 重置统计 */
void ParticleFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 系统重采样(低方差法) */
void ParticleFilter::resample()
{
    int n = m_particles.size();
    if (n == 0) return;

    QVector<double> cumWeights(n);
    cumWeights[0] = m_particles[0].weight;
    for (int i = 1; i < n; ++i) {
        cumWeights[i] = cumWeights[i - 1] + m_particles[i].weight;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0 / n);

    QVector<Particle> newParticles(n);
    double step = 1.0 / n;
    double pos = dist(gen);

    int idx = 0;
    for (int i = 0; i < n; ++i) {
        while (idx < n - 1 && cumWeights[idx] < pos) ++idx;
        newParticles[i].state = m_particles[idx].state;
        newParticles[i].weight = step;
        pos += step;
    }

    m_particles = std::move(newParticles);
}

/** @brief 归一化粒子权重 */
void ParticleFilter::normalizeWeights()
{
    double sum = 0.0;
    for (const auto& p : m_particles) sum += p.weight;
    if (sum > 1e-300) {
        for (auto& p : m_particles) p.weight /= sum;
    } else {
        double uniform = 1.0 / static_cast<double>(m_particles.size());
        for (auto& p : m_particles) p.weight = uniform;
    }
}
