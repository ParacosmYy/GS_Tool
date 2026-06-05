/**
 * @file algo_1225.cpp
 * @brief Algorithm module 1225
 */
#include "matrix1225/algo_1225.h"
QVector<double> algo_1225::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
