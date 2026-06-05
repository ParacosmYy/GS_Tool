/**
 * @file algo_1412.cpp
 * @brief Algorithm module 1412
 */
#include "compress1412/algo_1412.h"
QVector<double> algo_1412::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
