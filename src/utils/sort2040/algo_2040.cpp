/**
 * @file algo_2040.cpp
 * @brief Algorithm module 2040
 */
#include "sort2040/algo_2040.h"
QVector<double> algo_2040::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
