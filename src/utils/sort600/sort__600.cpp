/**
 * @file sort__600.cpp
 * @brief sort__600 implementation
 */
#include "sort600/sort__600.h"
QVector<double> sort__600::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

