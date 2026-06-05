/**
 * @file algo_1702.cpp
 * @brief Algorithm module 1702
 */
#include "poly1702/algo_1702.h"
QVector<double> algo_1702::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
