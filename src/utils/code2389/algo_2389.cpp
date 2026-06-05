/**
 * @file algo_2389.cpp
 * @brief Algorithm module 2389
 */
#include "code2389/algo_2389.h"
QVector<double> algo_2389::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
