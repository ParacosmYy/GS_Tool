/**
 * @file algo_1962.cpp
 * @brief Algorithm module 1962
 */
#include "poly1962/algo_1962.h"
QVector<double> algo_1962::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
