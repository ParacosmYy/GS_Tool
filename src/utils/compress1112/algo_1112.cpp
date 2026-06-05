/**
 * @file algo_1112.cpp
 * @brief Algorithm module 1112
 */
#include "compress1112/algo_1112.h"
QVector<double> algo_1112::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
