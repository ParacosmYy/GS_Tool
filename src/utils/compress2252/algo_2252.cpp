/**
 * @file algo_2252.cpp
 * @brief Algorithm module 2252
 */
#include "compress2252/algo_2252.h"
QVector<double> algo_2252::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
