/**
 * @file algo_2325.cpp
 * @brief Algorithm module 2325
 */
#include "matrix2325/algo_2325.h"
QVector<double> algo_2325::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
