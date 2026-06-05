/**
 * @file algo_2125.cpp
 * @brief Algorithm module 2125
 */
#include "matrix2125/algo_2125.h"
QVector<double> algo_2125::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
