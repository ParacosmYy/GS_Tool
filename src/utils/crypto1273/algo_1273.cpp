/**
 * @file algo_1273.cpp
 * @brief Algorithm module 1273
 */
#include "crypto1273/algo_1273.h"
QVector<double> algo_1273::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
