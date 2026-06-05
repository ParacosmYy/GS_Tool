/**
 * @file algo_2373.cpp
 * @brief Algorithm module 2373
 */
#include "crypto2373/algo_2373.h"
QVector<double> algo_2373::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
