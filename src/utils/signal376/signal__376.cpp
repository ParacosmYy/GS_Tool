/**
 * @file signal__376.cpp
 * @brief signal__376 implementation
 */
#include "signal376/signal__376.h"
QVector<double> signal__376::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

