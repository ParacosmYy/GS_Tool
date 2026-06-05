/**
 * @file algo_1293.cpp
 * @brief Algorithm module 1293
 */
#include "crypto1293/algo_1293.h"
QVector<double> algo_1293::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
