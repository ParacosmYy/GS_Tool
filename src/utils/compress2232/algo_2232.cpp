/**
 * @file algo_2232.cpp
 * @brief Algorithm module 2232
 */
#include "compress2232/algo_2232.h"
QVector<double> algo_2232::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
