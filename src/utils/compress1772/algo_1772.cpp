/**
 * @file algo_1772.cpp
 * @brief Algorithm module 1772
 */
#include "compress1772/algo_1772.h"
QVector<double> algo_1772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
