/**
 * @file signal__476.cpp
 * @brief signal__476 implementation
 */
#include "signal476/signal__476.h"
QVector<double> signal__476::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

