/**
 * @file algo_1472.cpp
 * @brief Algorithm module 1472
 */
#include "compress1472/algo_1472.h"
QVector<double> algo_1472::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
