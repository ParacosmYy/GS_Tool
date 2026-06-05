/**
 * @file algo_2014.cpp
 * @brief Algorithm module 2014
 */
#include "numeric2014/algo_2014.h"
QVector<double> algo_2014::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
