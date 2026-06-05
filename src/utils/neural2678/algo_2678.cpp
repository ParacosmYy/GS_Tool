/**
 * @file algo_2678.cpp
 * @brief Algorithm module 2678
 */
#include "neural2678/algo_2678.h"
QVector<double> algo_2678::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
