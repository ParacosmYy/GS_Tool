/**
 * @file algo_2552.cpp
 * @brief Algorithm module 2552
 */
#include "compress2552/algo_2552.h"
QVector<double> algo_2552::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
