/**
 * @file algo_2646.cpp
 * @brief Algorithm module 2646
 */
#include "signal2646/algo_2646.h"
QVector<double> algo_2646::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
