/**
 * @file algo_2377.cpp
 * @brief Algorithm module 2377
 */
#include "image2377/algo_2377.h"
QVector<double> algo_2377::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
