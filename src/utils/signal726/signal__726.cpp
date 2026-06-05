/**
 * @file signal__726.cpp
 * @brief signal__726 implementation
 */
#include "signal726/signal__726.h"
QVector<double> signal__726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

