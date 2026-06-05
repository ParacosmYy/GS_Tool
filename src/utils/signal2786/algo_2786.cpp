/**
 * @file algo_2786.cpp
 * @brief Algorithm module 2786
 */
#include "signal2786/algo_2786.h"
QVector<double> algo_2786::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
