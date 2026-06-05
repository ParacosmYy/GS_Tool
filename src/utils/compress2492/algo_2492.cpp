/**
 * @file algo_2492.cpp
 * @brief Algorithm module 2492
 */
#include "compress2492/algo_2492.h"
QVector<double> algo_2492::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
