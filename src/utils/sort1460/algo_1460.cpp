/**
 * @file algo_1460.cpp
 * @brief Algorithm module 1460
 */
#include "sort1460/algo_1460.h"
QVector<double> algo_1460::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
