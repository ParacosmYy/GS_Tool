/**
 * @file string__503.cpp
 * @brief string__503 implementation
 */
#include "string503/string__503.h"
QVector<double> string__503::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

