/**
 * @file algo_2497.cpp
 * @brief Algorithm module 2497
 */
#include "image2497/algo_2497.h"
QVector<double> algo_2497::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
