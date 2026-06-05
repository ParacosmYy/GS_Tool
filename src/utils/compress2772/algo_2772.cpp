/**
 * @file algo_2772.cpp
 * @brief Algorithm module 2772
 */
#include "compress2772/algo_2772.h"
QVector<double> algo_2772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
