/**
 * @file signal__456.cpp
 * @brief signal__456 implementation
 */
#include "signal456/signal__456.h"
QVector<double> signal__456::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

