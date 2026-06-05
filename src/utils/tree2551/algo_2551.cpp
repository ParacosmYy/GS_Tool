/**
 * @file algo_2551.cpp
 * @brief Algorithm module 2551
 */
#include "tree2551/algo_2551.h"
QVector<double> algo_2551::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
