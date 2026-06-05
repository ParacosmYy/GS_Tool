/**
 * @file algo_2592.cpp
 * @brief Algorithm module 2592
 */
#include "compress2592/algo_2592.h"
QVector<double> algo_2592::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
