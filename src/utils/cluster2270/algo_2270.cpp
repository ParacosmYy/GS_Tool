/**
 * @file algo_2270.cpp
 * @brief Algorithm module 2270
 */
#include "cluster2270/algo_2270.h"
QVector<double> algo_2270::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
