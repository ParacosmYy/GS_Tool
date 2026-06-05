/**
 * @file sort__440.cpp
 * @brief sort__440 implementation
 */
#include "sort440/sort__440.h"
QVector<double> sort__440::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

