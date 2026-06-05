/**
 * @file signal__346.cpp
 * @brief signal__346 implementation
 */
#include "signal346/signal__346.h"
QVector<double> signal__346::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

