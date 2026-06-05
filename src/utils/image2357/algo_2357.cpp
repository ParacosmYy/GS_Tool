/**
 * @file algo_2357.cpp
 * @brief Algorithm module 2357
 */
#include "image2357/algo_2357.h"
QVector<double> algo_2357::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
