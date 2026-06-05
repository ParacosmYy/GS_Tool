/**
 * @file string__393.cpp
 * @brief string__393 implementation
 */
#include "string393/string__393.h"
QVector<double> string__393::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

