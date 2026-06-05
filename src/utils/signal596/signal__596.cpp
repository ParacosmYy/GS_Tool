/**
 * @file signal__596.cpp
 * @brief signal__596 implementation
 */
#include "signal596/signal__596.h"
QVector<double> signal__596::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

