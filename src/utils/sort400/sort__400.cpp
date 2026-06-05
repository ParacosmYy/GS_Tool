/**
 * @file sort__400.cpp
 * @brief sort__400 implementation
 */
#include "sort400/sort__400.h"
QVector<double> sort__400::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

