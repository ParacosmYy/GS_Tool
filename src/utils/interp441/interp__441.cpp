/**
 * @file interp__441.cpp
 * @brief interp__441 implementation
 */
#include "interp441/interp__441.h"
QVector<double> interp__441::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

