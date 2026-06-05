/**
 * @file algo_2660.cpp
 * @brief Algorithm module 2660
 */
#include "sort2660/algo_2660.h"
QVector<double> algo_2660::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
