/**
 * @file algo_2021.cpp
 * @brief Algorithm module 2021
 */
#include "interp2021/algo_2021.h"
QVector<double> algo_2021::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
