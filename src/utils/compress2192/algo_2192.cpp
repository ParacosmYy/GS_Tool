/**
 * @file algo_2192.cpp
 * @brief Algorithm module 2192
 */
#include "compress2192/algo_2192.h"
QVector<double> algo_2192::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
