/**
 * @file algo_2397.cpp
 * @brief Algorithm module 2397
 */
#include "image2397/algo_2397.h"
QVector<double> algo_2397::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
