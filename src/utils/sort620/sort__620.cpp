/**
 * @file sort__620.cpp
 * @brief sort__620 implementation
 */
#include "sort620/sort__620.h"
QVector<double> sort__620::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}

