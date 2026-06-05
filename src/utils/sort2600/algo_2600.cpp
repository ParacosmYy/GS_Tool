/**
 * @file algo_2600.cpp
 * @brief Algorithm module 2600
 */
#include "sort2600/algo_2600.h"
QVector<double> algo_2600::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
