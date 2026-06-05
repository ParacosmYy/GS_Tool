/**
 * @file signal__746.cpp
 * @brief signal__746 implementation
 */
#include "signal746/signal__746.h"
QVector<double> signal__746::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

