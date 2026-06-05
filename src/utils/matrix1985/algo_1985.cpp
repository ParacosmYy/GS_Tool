/**
 * @file algo_1985.cpp
 * @brief Algorithm module 1985
 */
#include "matrix1985/algo_1985.h"
QVector<double> algo_1985::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
