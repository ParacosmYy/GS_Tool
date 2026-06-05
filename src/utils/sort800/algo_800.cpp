/**
 * @file algo_800.cpp
 * @brief Algorithm module 800
 */
#include "sort800/algo_800.h"
QVector<double> algo_800::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
