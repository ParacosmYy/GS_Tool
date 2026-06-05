/**
 * @file algo_1052.cpp
 * @brief Algorithm module 1052
 */
#include "compress1052/algo_1052.h"
QVector<double> algo_1052::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
