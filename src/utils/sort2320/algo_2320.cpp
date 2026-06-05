/**
 * @file algo_2320.cpp
 * @brief Algorithm module 2320
 */
#include "sort2320/algo_2320.h"
QVector<double> algo_2320::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
