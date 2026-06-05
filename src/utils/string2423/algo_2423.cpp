/**
 * @file algo_2423.cpp
 * @brief Algorithm module 2423
 */
#include "string2423/algo_2423.h"
QVector<double> algo_2423::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
