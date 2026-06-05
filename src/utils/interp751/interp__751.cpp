/**
 * @file interp__751.cpp
 * @brief interp__751 implementation
 */
#include "interp751/interp__751.h"
QVector<double> interp__751::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

