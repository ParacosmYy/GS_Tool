/**
 * @file algo_2310.cpp
 * @brief Algorithm module 2310
 */
#include "cluster2310/algo_2310.h"
QVector<double> algo_2310::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
