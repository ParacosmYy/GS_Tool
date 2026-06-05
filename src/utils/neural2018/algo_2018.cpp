/**
 * @file algo_2018.cpp
 * @brief Algorithm module 2018
 */
#include "neural2018/algo_2018.h"
QVector<double> algo_2018::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
