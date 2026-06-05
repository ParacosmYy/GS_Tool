/**
 * @file algo_2702.cpp
 * @brief Algorithm module 2702
 */
#include "poly2702/algo_2702.h"
QVector<double> algo_2702::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
