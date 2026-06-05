/**
 * @file algo_2400.cpp
 * @brief Algorithm module 2400
 */
#include "sort2400/algo_2400.h"
QVector<double> algo_2400::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
