/**
 * @file algo_1965.cpp
 * @brief Algorithm module 1965
 */
#include "matrix1965/algo_1965.h"
QVector<double> algo_1965::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
