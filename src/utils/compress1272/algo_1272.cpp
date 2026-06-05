/**
 * @file algo_1272.cpp
 * @brief Algorithm module 1272
 */
#include "compress1272/algo_1272.h"
QVector<double> algo_1272::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
