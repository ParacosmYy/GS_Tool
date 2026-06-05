/**
 * @file algo_2105.cpp
 * @brief Algorithm module 2105
 */
#include "matrix2105/algo_2105.h"
QVector<double> algo_2105::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
