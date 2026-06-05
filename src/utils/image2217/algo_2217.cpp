/**
 * @file algo_2217.cpp
 * @brief Algorithm module 2217
 */
#include "image2217/algo_2217.h"
QVector<double> algo_2217::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
