/**
 * @file algo_2537.cpp
 * @brief Algorithm module 2537
 */
#include "image2537/algo_2537.h"
QVector<double> algo_2537::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
