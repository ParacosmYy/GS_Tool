/**
 * @file algo_2760.cpp
 * @brief Algorithm module 2760
 */
#include "sort2760/algo_2760.h"
QVector<double> algo_2760::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
