/**
 * @file algo_2457.cpp
 * @brief Algorithm module 2457
 */
#include "image2457/algo_2457.h"
QVector<double> algo_2457::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
