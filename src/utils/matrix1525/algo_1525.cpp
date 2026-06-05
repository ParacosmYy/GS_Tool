/**
 * @file algo_1525.cpp
 * @brief Algorithm module 1525
 */
#include "matrix1525/algo_1525.h"
QVector<double> algo_1525::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
