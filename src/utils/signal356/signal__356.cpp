/**
 * @file signal__356.cpp
 * @brief signal__356 implementation
 */
#include "signal356/signal__356.h"
QVector<double> signal__356::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

