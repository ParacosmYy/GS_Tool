/**
 * @file algo_1177.cpp
 * @brief Algorithm module 1177
 */
#include "image1177/algo_1177.h"
QVector<double> algo_1177::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
