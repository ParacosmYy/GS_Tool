/**
 * @file interp__521.cpp
 * @brief interp__521 implementation
 */
#include "interp521/interp__521.h"
QVector<double> interp__521::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

