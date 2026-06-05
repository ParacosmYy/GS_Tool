/**
 * @file sort__520.cpp
 * @brief sort__520 implementation
 */
#include "sort520/sort__520.h"
QVector<double> sort__520::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

