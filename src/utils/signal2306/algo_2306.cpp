/**
 * @file algo_2306.cpp
 * @brief Algorithm module 2306
 */
#include "signal2306/algo_2306.h"
QVector<double> algo_2306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
