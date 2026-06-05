/**
 * @file algo_1992.cpp
 * @brief Algorithm module 1992
 */
#include "compress1992/algo_1992.h"
QVector<double> algo_1992::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
