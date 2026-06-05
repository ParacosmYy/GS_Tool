/**
 * @file algo_2425.cpp
 * @brief Algorithm module 2425
 */
#include "matrix2425/algo_2425.h"
QVector<double> algo_2425::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
