/**
 * @file algo_2560.cpp
 * @brief Algorithm module 2560
 */
#include "sort2560/algo_2560.h"
QVector<double> algo_2560::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
