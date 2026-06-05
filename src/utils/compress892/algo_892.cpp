/**
 * @file algo_892.cpp
 * @brief Algorithm module 892
 */
#include "compress892/algo_892.h"
QVector<double> algo_892::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
