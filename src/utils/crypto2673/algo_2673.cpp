/**
 * @file algo_2673.cpp
 * @brief Algorithm module 2673
 */
#include "crypto2673/algo_2673.h"
QVector<double> algo_2673::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
