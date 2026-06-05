/**
 * @file algo_1172.cpp
 * @brief Algorithm module 1172
 */
#include "compress1172/algo_1172.h"
QVector<double> algo_1172::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
