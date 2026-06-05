/**
 * @file algo_2725.cpp
 * @brief Algorithm module 2725
 */
#include "matrix2725/algo_2725.h"
QVector<double> algo_2725::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
