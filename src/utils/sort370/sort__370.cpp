/**
 * @file sort__370.cpp
 * @brief sort__370 implementation
 */
#include "sort370/sort__370.h"
QVector<double> sort__370::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

