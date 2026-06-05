/**
 * @file algo_840.cpp
 * @brief Algorithm module 840
 */
#include "sort840/algo_840.h"
QVector<double> algo_840::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
