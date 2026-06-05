/**
 * @file algo_2453.cpp
 * @brief Algorithm module 2453
 */
#include "crypto2453/algo_2453.h"
QVector<double> algo_2453::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
