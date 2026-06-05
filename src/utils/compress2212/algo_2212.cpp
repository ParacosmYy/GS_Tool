/**
 * @file algo_2212.cpp
 * @brief Algorithm module 2212
 */
#include "compress2212/algo_2212.h"
QVector<double> algo_2212::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
