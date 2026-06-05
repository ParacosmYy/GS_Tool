/**
 * @file algo_2480.cpp
 * @brief Algorithm module 2480
 */
#include "sort2480/algo_2480.h"
QVector<double> algo_2480::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
