/**
 * @file algo_2052.cpp
 * @brief Algorithm module 2052
 */
#include "compress2052/algo_2052.h"
QVector<double> algo_2052::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
