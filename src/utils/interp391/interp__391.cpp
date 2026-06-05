/**
 * @file interp__391.cpp
 * @brief interp__391 implementation
 */
#include "interp391/interp__391.h"
QVector<double> interp__391::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

