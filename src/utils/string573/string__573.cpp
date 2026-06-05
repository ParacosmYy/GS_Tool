/**
 * @file string__573.cpp
 * @brief string__573 implementation
 */
#include "string573/string__573.h"
QVector<double> string__573::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

