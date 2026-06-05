/**
 * @file signal__426.cpp
 * @brief signal__426 implementation
 */
#include "signal426/signal__426.h"
QVector<double> signal__426::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

