/**
 * @file algo_2752.cpp
 * @brief Algorithm module 2752
 */
#include "compress2752/algo_2752.h"
QVector<double> algo_2752::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
