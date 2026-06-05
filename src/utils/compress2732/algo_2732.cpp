/**
 * @file algo_2732.cpp
 * @brief Algorithm module 2732
 */
#include "compress2732/algo_2732.h"
QVector<double> algo_2732::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
