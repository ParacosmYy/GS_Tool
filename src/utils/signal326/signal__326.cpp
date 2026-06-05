/**
 * @file signal__326.cpp
 * @brief signal__326 implementation
 */
#include "signal326/signal__326.h"
QVector<double> signal__326::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

