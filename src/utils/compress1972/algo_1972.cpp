/**
 * @file algo_1972.cpp
 * @brief Algorithm module 1972
 */
#include "compress1972/algo_1972.h"
QVector<double> algo_1972::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
