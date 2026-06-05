/**
 * @file algo_1232.cpp
 * @brief Algorithm module 1232
 */
#include "compress1232/algo_1232.h"
QVector<double> algo_1232::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
