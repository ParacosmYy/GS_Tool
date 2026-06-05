/**
 * @file signal__606.cpp
 * @brief signal__606 implementation
 */
#include "signal606/signal__606.h"
QVector<double> signal__606::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

