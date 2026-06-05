/**
 * @file algo_1300.cpp
 * @brief Algorithm module 1300
 */
#include "sort1300/algo_1300.h"
QVector<double> algo_1300::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
