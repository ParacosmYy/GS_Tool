/**
 * @file algo_1617.cpp
 * @brief Algorithm module 1617
 */
#include "image1617/algo_1617.h"
QVector<double> algo_1617::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
