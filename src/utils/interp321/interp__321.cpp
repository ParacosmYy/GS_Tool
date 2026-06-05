/**
 * @file interp__321.cpp
 * @brief interp__321 implementation
 */
#include "interp321/interp__321.h"
QVector<double> interp__321::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

