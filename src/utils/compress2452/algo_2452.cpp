/**
 * @file algo_2452.cpp
 * @brief Algorithm module 2452
 */
#include "compress2452/algo_2452.h"
QVector<double> algo_2452::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
