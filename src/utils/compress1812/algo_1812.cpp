/**
 * @file algo_1812.cpp
 * @brief Algorithm module 1812
 */
#include "compress1812/algo_1812.h"
QVector<double> algo_1812::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
