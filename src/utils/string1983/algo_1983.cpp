/**
 * @file algo_1983.cpp
 * @brief Algorithm module 1983
 */
#include "string1983/algo_1983.h"
QVector<double> algo_1983::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
