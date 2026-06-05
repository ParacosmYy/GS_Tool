/**
 * @file algo_2213.cpp
 * @brief Algorithm module 2213
 */
#include "crypto2213/algo_2213.h"
QVector<double> algo_2213::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
