/**
 * @file algo_2262.cpp
 * @brief Algorithm module 2262
 */
#include "poly2262/algo_2262.h"
QVector<double> algo_2262::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
