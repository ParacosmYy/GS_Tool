/**
 * @file algo_871.cpp
 * @brief Algorithm module 871
 */
#include "tree871/algo_871.h"
QVector<double> algo_871::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
