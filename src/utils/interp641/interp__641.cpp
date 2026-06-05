/**
 * @file interp__641.cpp
 * @brief interp__641 implementation
 */
#include "interp641/interp__641.h"
QVector<double> interp__641::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

