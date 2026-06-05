/**
 * @file algo_2082.cpp
 * @brief Algorithm module 2082
 */
#include "poly2082/algo_2082.h"
QVector<double> algo_2082::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
