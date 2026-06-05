/**
 * @file algo_1312.cpp
 * @brief Algorithm module 1312
 */
#include "compress1312/algo_1312.h"
QVector<double> algo_1312::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
