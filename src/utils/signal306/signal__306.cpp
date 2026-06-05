/**
 * @file signal__306.cpp
 * @brief signal__306 implementation
 */
#include "signal306/signal__306.h"
QVector<double> signal__306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

