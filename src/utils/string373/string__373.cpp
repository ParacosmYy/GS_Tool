/**
 * @file string__373.cpp
 * @brief string__373 implementation
 */
#include "string373/string__373.h"
QVector<double> string__373::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

