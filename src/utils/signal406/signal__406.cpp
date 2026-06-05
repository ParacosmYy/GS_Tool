/**
 * @file signal__406.cpp
 * @brief signal__406 implementation
 */
#include "signal406/signal__406.h"
QVector<double> signal__406::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

