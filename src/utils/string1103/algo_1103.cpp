/**
 * @file algo_1103.cpp
 * @brief Algorithm module 1103
 */
#include "string1103/algo_1103.h"
QVector<double> algo_1103::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
