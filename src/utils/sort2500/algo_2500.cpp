/**
 * @file algo_2500.cpp
 * @brief Algorithm module 2500
 */
#include "sort2500/algo_2500.h"
QVector<double> algo_2500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
