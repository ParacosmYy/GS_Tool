/**
 * @file algo_2010.cpp
 * @brief Algorithm module 2010
 */
#include "cluster2010/algo_2010.h"
QVector<double> algo_2010::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
