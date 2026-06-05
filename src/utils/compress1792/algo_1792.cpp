/**
 * @file algo_1792.cpp
 * @brief Algorithm module 1792
 */
#include "compress1792/algo_1792.h"
QVector<double> algo_1792::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
