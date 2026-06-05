/**
 * @file algo_1080.cpp
 * @brief Algorithm module 1080
 */
#include "sort1080/algo_1080.h"
QVector<double> algo_1080::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
