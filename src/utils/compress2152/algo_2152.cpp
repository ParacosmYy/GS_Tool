/**
 * @file algo_2152.cpp
 * @brief Algorithm module 2152
 */
#include "compress2152/algo_2152.h"
QVector<double> algo_2152::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
