/**
 * @file algo_1236.cpp
 * @brief Algorithm module 1236
 */
#include "geometry1236/algo_1236.h"
QVector<double> algo_1236::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
