/**
 * @file algo_1040.cpp
 * @brief Algorithm module 1040
 */
#include "sort1040/algo_1040.h"
QVector<double> algo_1040::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
