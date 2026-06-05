/**
 * @file algo_2293.cpp
 * @brief Algorithm module 2293
 */
#include "crypto2293/algo_2293.h"
QVector<double> algo_2293::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
