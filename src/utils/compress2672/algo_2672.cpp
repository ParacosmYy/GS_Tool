/**
 * @file algo_2672.cpp
 * @brief Algorithm module 2672
 */
#include "compress2672/algo_2672.h"
QVector<double> algo_2672::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
