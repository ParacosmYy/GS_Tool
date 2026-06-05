/**
 * @file signal__396.cpp
 * @brief signal__396 implementation
 */
#include "signal396/signal__396.h"
QVector<double> signal__396::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

