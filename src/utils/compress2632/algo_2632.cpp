/**
 * @file algo_2632.cpp
 * @brief Algorithm module 2632
 */
#include "compress2632/algo_2632.h"
QVector<double> algo_2632::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
