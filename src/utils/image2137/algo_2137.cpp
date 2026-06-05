/**
 * @file algo_2137.cpp
 * @brief Algorithm module 2137
 */
#include "image2137/algo_2137.h"
QVector<double> algo_2137::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
