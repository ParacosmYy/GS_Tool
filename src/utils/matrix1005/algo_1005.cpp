/**
 * @file algo_1005.cpp
 * @brief Algorithm module 1005
 */
#include "matrix1005/algo_1005.h"
QVector<double> algo_1005::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
