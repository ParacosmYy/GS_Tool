/**
 * @file interp__301.cpp
 * @brief interp__301 implementation
 */
#include "interp301/interp__301.h"
QVector<double> interp__301::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

