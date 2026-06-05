/**
 * @file algo_2381.cpp
 * @brief Algorithm module 2381
 */
#include "interp2381/algo_2381.h"
QVector<double> algo_2381::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
