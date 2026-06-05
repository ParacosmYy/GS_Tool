/**
 * @file algo_2022.cpp
 * @brief Algorithm module 2022
 */
#include "poly2022/algo_2022.h"
QVector<double> algo_2022::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
