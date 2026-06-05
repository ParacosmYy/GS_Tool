/**
 * @file PidController.cpp
 * @brief PID控制器实现
 */

#include "utils/pid/PidController.h"
#include <QElapsedTimer>
#include <QtMath>

PidController::PidController(QObject* parent)
    : QObject(parent), m_mode(Mode::Positional),
      m_setpoint(0.0), m_integral(0.0), m_prevError(0.0),
      m_prevOutput(0.0), m_integralLimit(1000.0),
      m_derivFilterAlpha(0.1), m_filteredDerivative(0.0),
      m_firstUpdate(true), m_timeSum(0.0) {}

void PidController::setParams(const PidParams& params) { m_params = params; }
void PidController::setMode(Mode m) { m_mode = m; }
void PidController::setSetpoint(double sp) { m_setpoint = sp; emit setpointChanged(sp); }
void PidController::setIntegralLimit(double limit) { m_integralLimit = qMax(0.0, limit); }
void PidController::setDerivativeFilter(double alpha) { m_derivFilterAlpha = qBound(0.0, alpha, 1.0); }

double PidController::update(double pv, double dtMs)
{
    QElapsedTimer timer;
    timer.start();

    double error = m_setpoint - pv;
    double dt = dtMs / 1000.0;
    if (dt <= 0.0) dt = 0.001;

    double output = 0.0;

    if (m_mode == Mode::Positional) {
        /* 位置式PID */
        m_integral += error * dt;

        /* 抗积分饱和 */
        m_integral = qBound(-m_integralLimit, m_integral, m_integralLimit);

        double derivative = (error - m_prevError) / dt;
        m_filteredDerivative = m_derivFilterAlpha * derivative +
                               (1.0 - m_derivFilterAlpha) * m_filteredDerivative;

        output = m_params.kp * error +
                 m_params.ki * m_integral +
                 m_params.kd * m_filteredDerivative;
    } else {
        /* 增量式PID */
        double deltaIntegral = error * dt;
        double derivative = (error - m_prevError) / dt;
        m_filteredDerivative = m_derivFilterAlpha * derivative +
                               (1.0 - m_derivFilterAlpha) * m_filteredDerivative;

        double deltaOutput = m_params.kp * (error - m_prevError) +
                             m_params.ki * deltaIntegral +
                             m_params.kd * (m_filteredDerivative);
        output = m_prevOutput + deltaOutput;
    }

    /* 输出限幅 */
    output = qBound(m_params.outputMin, output, m_params.outputMax);

    m_prevError = error;
    m_prevOutput = output;

    /* 统计更新 */
    double elapsed = timer.elapsed();
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    if (m_stats.totalUpdates > 10) {
        m_stats.steadyStateError = error;
    }

    emit outputComputed(output, error);
    return output;
}

void PidController::reset()
{
    m_integral = 0.0;
    m_prevError = 0.0;
    m_prevOutput = 0.0;
    m_filteredDerivative = 0.0;
    m_firstUpdate = true;
}

void PidController::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
