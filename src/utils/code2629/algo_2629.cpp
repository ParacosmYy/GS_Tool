/**
 * @file algo_2629.cpp
 * @brief Algorithm module 2629
 */
#include "code2629/algo_2629.h"
QVector<double> algo_2629::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
