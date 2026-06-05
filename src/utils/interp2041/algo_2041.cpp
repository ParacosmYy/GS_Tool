/**
 * @file algo_2041.cpp
 * @brief Algorithm module 2041
 */
#include "interp2041/algo_2041.h"
QVector<double> algo_2041::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
