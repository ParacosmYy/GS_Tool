/**
 * @file algo_2097.cpp
 * @brief Algorithm module 2097
 */
#include "image2097/algo_2097.h"
QVector<double> algo_2097::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
