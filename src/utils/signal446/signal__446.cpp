/**
 * @file signal__446.cpp
 * @brief signal__446 implementation
 */
#include "signal446/signal__446.h"
QVector<double> signal__446::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

