/**
 * @file algo_1280.cpp
 * @brief Algorithm module 1280
 */
#include "sort1280/algo_1280.h"
QVector<double> algo_1280::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
