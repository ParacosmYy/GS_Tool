/**
 * @file algo_1252.cpp
 * @brief Algorithm module 1252
 */
#include "compress1252/algo_1252.h"
QVector<double> algo_1252::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
