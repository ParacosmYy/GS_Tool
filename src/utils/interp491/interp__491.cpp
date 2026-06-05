/**
 * @file interp__491.cpp
 * @brief interp__491 implementation
 */
#include "interp491/interp__491.h"
QVector<double> interp__491::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

