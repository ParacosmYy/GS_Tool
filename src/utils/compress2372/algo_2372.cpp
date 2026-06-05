/**
 * @file algo_2372.cpp
 * @brief Algorithm module 2372
 */
#include "compress2372/algo_2372.h"
QVector<double> algo_2372::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
