/**
 * @file algo_2237.cpp
 * @brief Algorithm module 2237
 */
#include "image2237/algo_2237.h"
QVector<double> algo_2237::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
