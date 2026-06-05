/**
 * @file algo_2572.cpp
 * @brief Algorithm module 2572
 */
#include "compress2572/algo_2572.h"
QVector<double> algo_2572::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
