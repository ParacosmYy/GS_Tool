/**
 * @file PidSimulator.cpp
 * @brief PID仿真引擎实现 — 闭环仿真、一阶被控对象、性能指标计算
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/pid/PidSimulator.h"

#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/* ============================================================
 * 构造
 * ============================================================ */

PidSimulator::PidSimulator(QObject* parent)
    : QObject(parent)
    , m_plantState(0.0)
    , m_integral(0.0)
    , m_prevError(0.0)
    , m_filteredDeriv(0.0)
    , m_delayIndex(0)
{
}

/* ============================================================
 * 参数配置
 * ============================================================ */

void PidSimulator::setPlantParams(const PlantParams& params)
{
    m_plant = params;
}

void PidSimulator::setSimConfig(const SimConfig& config)
{
    m_config = config;
}

/* ============================================================
 * 核心: 完整仿真
 * ============================================================ */

PidResponse PidSimulator::simulate(const PidParams& pid, PidResponseType type)
{
    const double dt = qMax(pid.sampleTime, m_config.sampleTime);
    const int steps = static_cast<int>(m_config.duration / dt);
    if (steps <= 0) {
        return PidResponse{};
    }

    /* 重置运行时状态 */
    m_plantState    = 0.0;
    m_integral      = 0.0;
    m_prevError     = 0.0;
    m_filteredDeriv = 0.0;

    /* 初始化纯滞后缓冲区 */
    const int delaySteps = static_cast<int>(m_plant.delay / dt);
    m_delayBuffer.fill(0.0, qMax(delaySteps, 1));
    m_delayIndex = 0;

    /* 生成设定值序列 */
    const QVector<double> setpoints = generateSetpoint(type, steps);

    /* 准备输出容器 */
    PidResponse resp;
    resp.timePoints.reserve(steps);
    resp.setpointData.reserve(steps);
    resp.processData.reserve(steps);
    resp.outputData.reserve(steps);
    resp.errorData.reserve(steps);

    double iae = 0.0;

    for (int i = 0; i < steps; ++i) {
        const double sp  = setpoints.value(i, 0.0);
        const double pv  = m_plantState;
        const double error = sp - pv;

        /* ---- PID 计算 (位置式 + 抗积分饱和) ---- */
        m_integral += error * dt;
        m_integral  = qBound(-m_config.windupLimit, m_integral, m_config.windupLimit);

        double derivative = (i > 0) ? (error - m_prevError) / dt : 0.0;
        m_filteredDeriv   = m_config.derivFilterAlpha * derivative
                          + (1.0 - m_config.derivFilterAlpha) * m_filteredDeriv;

        double control = pid.kp * error
                       + pid.ki * m_integral
                       + pid.kd * m_filteredDeriv;

        /* 输出限幅 */
        control = qBound(pid.outputMin, control, pid.outputMax);

        m_prevError = error;

        /* ---- 被控对象更新 ---- */
        const double delayedControl = updatePlant(control);

        /* 一阶惯性: T * dy/dt + y = K * u */
        const double tau = qMax(m_plant.timeConstant, 1e-6);
        m_plantState += (m_plant.gain * delayedControl - m_plantState) * (dt / tau);

        /* 测量噪声注入 */
        double measuredPv = m_plantState;
        if (m_plant.noiseStddev > 0.0) {
            measuredPv += QRandomGenerator::global()->generateDouble() * 2.0 * m_plant.noiseStddev
                        - m_plant.noiseStddev;
        }

        /* 绝对误差积分 */
        iae += qAbs(error) * dt;

        /* 记录数据 */
        resp.timePoints.append(i * dt);
        resp.setpointData.append(sp);
        resp.processData.append(measuredPv);
        resp.outputData.append(control);
        resp.errorData.append(error);
    }

    resp.integralAbsoluteError = iae;

    /* 计算性能指标 */
    computeMetrics(resp, setpoints.last());

    /* 更新统计 */
    ++m_stats.simulationsRun;
    m_stats.stepsComputed += static_cast<quint64>(steps);

    emit simulationComplete(resp.processData, resp.outputData);
    return resp;
}

/* ============================================================
 * 被控对象一步更新 (含纯滞后)
 * ============================================================ */

double PidSimulator::updatePlant(double controlOutput)
{
    if (m_delayBuffer.isEmpty()) {
        return controlOutput;
    }

    m_delayBuffer[m_delayIndex] = controlOutput;
    m_delayIndex = (m_delayIndex + 1) % m_delayBuffer.size();

    /* 读取延迟后的输出 (最旧的数据) */
    const int readIdx = m_delayIndex;
    return m_delayBuffer[readIdx];
}

/* ============================================================
 * 生成激励信号序列
 * ============================================================ */

QVector<double> PidSimulator::generateSetpoint(PidResponseType type, int steps) const
{
    QVector<double> sp(steps, 0.0);
    const double dt = qMax(m_config.sampleTime, 0.001);

    for (int i = 0; i < steps; ++i) {
        const double t = i * dt;
        switch (type) {
        case PidResponseType::Step:
            sp[i] = 1.0;
            break;
        case PidResponseType::Ramp:
            sp[i] = qMin(t / m_config.duration, 1.0);
            break;
        case PidResponseType::Sine:
            sp[i] = 0.5 * (1.0 + qSin(2.0 * M_PI * 0.5 * t));
            break;
        case PidResponseType::Square:
            sp[i] = (qFmod(t, 2.0) < 1.0) ? 1.0 : 0.0;
            break;
        case PidResponseType::Impulse:
            sp[i] = (i == 0) ? 1.0 : 0.0;
            break;
        }
    }
    return sp;
}

/* ============================================================
 * 性能指标计算
 * ============================================================ */

void PidSimulator::computeMetrics(PidResponse& resp, double setpoint) const
{
    if (resp.processData.isEmpty() || resp.setpointData.isEmpty()) {
        return;
    }

    const int n = resp.processData.size();
    const double sp = resp.setpointData.last();
    const double tolerance = qAbs(sp) * 0.02 + 1e-6; /* 2% 带 */
    const double dt = (n > 1) ? (resp.timePoints[1] - resp.timePoints[0]) : 0.01;

    /* 上升时间: 首次从 10% 到 90% 的时间差 */
    const double lo = sp * 0.10;
    const double hi = sp * 0.90;
    int riseStart = -1;
    int riseEnd   = -1;

    for (int i = 0; i < n; ++i) {
        if (riseStart < 0 && resp.processData[i] >= lo) { riseStart = i; }
        if (riseEnd < 0 && resp.processData[i] >= hi)   { riseEnd = i; break; }
    }

    if (riseStart >= 0 && riseEnd >= 0) {
        resp.riseTime = (riseEnd - riseStart) * dt;
    }

    /* 超调量 */
    double peakVal = *std::max_element(resp.processData.constBegin(),
                                       resp.processData.constEnd());
    if (sp > 0.0 && peakVal > sp) {
        resp.overshoot = (peakVal - sp) / sp * 100.0;
    }

    /* 调节时间: 最后一次离开 ±2% 带的时间 */
    resp.settlingTime = resp.timePoints.last();
    for (int i = n - 1; i >= 0; --i) {
        if (qAbs(resp.processData[i] - sp) > tolerance) {
            resp.settlingTime = (i < n - 1) ? resp.timePoints[i + 1]
                                            : resp.timePoints[i];
            break;
        }
    }

    /* 稳态误差: 最后 10% 样本平均 */
    const int tailStart = static_cast<int>(n * 0.9);
    double errSum = 0.0;
    int errCount  = 0;
    for (int i = tailStart; i < n; ++i) {
        errSum += resp.setpointData[i] - resp.processData[i];
        ++errCount;
    }
    resp.steadyStateError = (errCount > 0) ? (errSum / errCount) : 0.0;
}

/* resetStatistics() 定义在 PidSimulatorStats.cpp */
