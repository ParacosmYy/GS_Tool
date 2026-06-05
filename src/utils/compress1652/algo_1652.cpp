/**
 * @file algo_1652.cpp
 * @brief Algorithm module 1652
 */
#include "compress1652/algo_1652.h"
QVector<double> algo_1652::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
