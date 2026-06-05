/**
 * @file HysteresisFilter.cpp
 * @brief 滞后滤波器实现
 */

#include "utils/hysteresis/HysteresisFilter.h"
#include <QElapsedTimer>

HysteresisFilter::HysteresisFilter(QObject* parent)
    : QObject(parent), m_upperThreshold(0.7), m_lowerThreshold(0.3),
      m_debounceTimeMs(0.0), m_currentValue(0.0),
      m_state(State::Unknown), m_debounceTimer(0.0),
      m_pendingState(State::Unknown), m_timeSum(0.0) {}

void HysteresisFilter::setUpperThreshold(double t) { m_upperThreshold = t; }
void HysteresisFilter::setLowerThreshold(double t) { m_lowerThreshold = t; }
void HysteresisFilter::setDebounceTimeMs(double ms) { m_debounceTimeMs = qMax(0.0, ms); }

HysteresisFilter::State HysteresisFilter::update(double value, double dtMs)
{
    QElapsedTimer timer;
    timer.start();

    m_currentValue = value;
    State newState = m_state;

    /* 滞后逻辑 */
    if (value >= m_upperThreshold) {
        newState = State::High;
    } else if (value <= m_lowerThreshold) {
        newState = State::Low;
    }
    /* 在上下阈值之间时保持当前状态 */

    /* 去抖处理 */
    if (m_debounceTimeMs > 0.0) {
        if (newState != m_state) {
            if (newState != m_pendingState) {
                m_pendingState = newState;
                m_debounceTimer = 0.0;
            }
            m_debounceTimer += dtMs;
            if (m_debounceTimer >= m_debounceTimeMs) {
                /* 确认状态切换 */
                State oldState = m_state;
                m_state = m_pendingState;
                ++m_stats.totalStateChanges;
                if (dtMs > 0.0) {
                    if (oldState == State::High) m_stats.lowDurationMs += dtMs;
                    else m_stats.highDurationMs += dtMs;
                }
                emit stateChanged(m_state, oldState);
                m_debounceTimer = 0.0;
                m_pendingState = State::Unknown;
            } else {
                /* 等待去抖，过滤抖动 */
                ++m_stats.totalBouncesFiltered;
                emit bounceFiltered(value);
            }
        } else {
            m_debounceTimer = 0.0;
            m_pendingState = State::Unknown;
        }
    } else {
        /* 无去抖，直接切换 */
        if (newState != m_state && newState != State::Unknown) {
            State oldState = m_state;
            m_state = newState;
            ++m_stats.totalStateChanges;
            if (dtMs > 0.0) {
                if (oldState == State::High) m_stats.lowDurationMs += dtMs;
                else m_stats.highDurationMs += dtMs;
            }
            emit stateChanged(m_state, oldState);
        }
    }

    ++m_stats.totalUpdates;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    return m_state;
}

HysteresisFilter::State HysteresisFilter::currentState() const { return m_state; }
double HysteresisFilter::currentValue() const { return m_currentValue; }
bool HysteresisFilter::isHigh() const { return m_state == State::High; }
bool HysteresisFilter::isLow() const { return m_state == State::Low; }

void HysteresisFilter::reset()
{
    m_state = State::Unknown;
    m_currentValue = 0.0;
    m_debounceTimer = 0.0;
    m_pendingState = State::Unknown;
}

void HysteresisFilter::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
