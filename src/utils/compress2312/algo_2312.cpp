/**
 * @file algo_2312.cpp
 * @brief Algorithm module 2312
 */
#include "compress2312/algo_2312.h"
QVector<double> algo_2312::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
