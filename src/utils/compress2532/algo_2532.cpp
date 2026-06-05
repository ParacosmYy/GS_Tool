/**
 * @file algo_2532.cpp
 * @brief Algorithm module 2532
 */
#include "compress2532/algo_2532.h"
QVector<double> algo_2532::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
