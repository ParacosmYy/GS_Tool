/**
 * @file algo_1397.cpp
 * @brief Algorithm module 1397
 */
#include "image1397/algo_1397.h"
QVector<double> algo_1397::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
