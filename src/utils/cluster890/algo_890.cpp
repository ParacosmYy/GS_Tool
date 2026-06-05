/**
 * @file algo_890.cpp
 * @brief Algorithm module 890
 */
#include "cluster890/algo_890.h"
QVector<double> algo_890::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
