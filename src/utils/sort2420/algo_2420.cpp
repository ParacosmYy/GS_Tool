/**
 * @file algo_2420.cpp
 * @brief Algorithm module 2420
 */
#include "sort2420/algo_2420.h"
QVector<double> algo_2420::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
