/**
 * @file KalmanFilter1D.cpp
 * @brief 一维卡尔曼滤波器实现 — 预测+更新循环
 */

#include "utils/kalman/KalmanFilter1D.h"

#include <QtMath>

KalmanFilter1D::KalmanFilter1D(QObject* parent)
    : QObject(parent)
    , m_state(0.0)
    , m_error(1.0)
    , m_q(0.01)
    , m_r(0.1)
    , m_gain(0.0)
    , m_innovation(0.0)
    , m_estSum(0.0)
{
}

void KalmanFilter1D::initialize(double initialState, double initialError)
{
    m_state = initialState;
    m_error = initialError;
    m_gain = 0.0;
    ++m_stats.totalResets;
}

void KalmanFilter1D::setProcessNoise(double q) { m_q = qMax(0.0, q); }
void KalmanFilter1D::setMeasurementNoise(double r) { m_r = qMax(1e-10, r); }

/** @brief 预测步骤 */
void KalmanFilter1D::predict()
{
    /* 状态不变(匀速模型), 误差增加过程噪声 */
    m_error += m_q;
}

/** @brief 更新步骤 @param measurement 测量值 @return 估计 */
double KalmanFilter1D::update(double measurement)
{
    /* 预测 */
    predict();

    /* 计算卡尔曼增益 */
    m_gain = m_error / (m_error + m_r);

    /* 计算新息 */
    m_innovation = measurement - m_state;
    if (qAbs(m_innovation) > m_stats.peakInnovation) {
        m_stats.peakInnovation = qAbs(m_innovation);
    }

    /* 更新状态估计 */
    m_state = m_state + m_gain * m_innovation;

    /* 更新误差协方差 */
    m_error = (1.0 - m_gain) * m_error;

    /* 统计 */
    ++m_stats.totalUpdates;
    m_estSum += m_state;
    m_stats.averageEstimate = m_estSum
        / static_cast<double>(m_stats.totalUpdates);

    emit updated(measurement, m_state);
    return m_state;
}

void KalmanFilter1D::resetStatistics()
{
    m_stats = Stats{};
    m_estSum = 0.0;
}
