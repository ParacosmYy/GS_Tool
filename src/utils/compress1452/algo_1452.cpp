/**
 * @file algo_1452.cpp
 * @brief Algorithm module 1452
 */
#include "compress1452/algo_1452.h"
QVector<double> algo_1452::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
