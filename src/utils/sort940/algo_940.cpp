/**
 * @file algo_940.cpp
 * @brief Algorithm module 940
 */
#include "sort940/algo_940.h"
QVector<double> algo_940::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
