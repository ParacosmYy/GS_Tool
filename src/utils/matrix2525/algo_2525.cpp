/**
 * @file algo_2525.cpp
 * @brief Algorithm module 2525
 */
#include "matrix2525/algo_2525.h"
QVector<double> algo_2525::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
