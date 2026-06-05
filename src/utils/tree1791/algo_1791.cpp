/**
 * @file algo_1791.cpp
 * @brief Algorithm module 1791
 */
#include "tree1791/algo_1791.h"
QVector<double> algo_1791::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
