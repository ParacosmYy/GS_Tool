/**
 * @file algo_2440.cpp
 * @brief Algorithm module 2440
 */
#include "sort2440/algo_2440.h"
QVector<double> algo_2440::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
