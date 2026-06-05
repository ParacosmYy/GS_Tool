/**
 * @file algo_1952.cpp
 * @brief Algorithm module 1952
 */
#include "compress1952/algo_1952.h"
QVector<double> algo_1952::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
