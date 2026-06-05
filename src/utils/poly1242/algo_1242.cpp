/**
 * @file algo_1242.cpp
 * @brief Algorithm module 1242
 */
#include "poly1242/algo_1242.h"
QVector<double> algo_1242::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
