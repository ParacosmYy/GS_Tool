/**
 * @file algo_2019.cpp
 * @brief Algorithm module 2019
 */
#include "quantum2019/algo_2019.h"
QVector<double> algo_2019::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
