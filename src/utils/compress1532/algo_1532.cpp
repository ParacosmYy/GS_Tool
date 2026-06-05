/**
 * @file algo_1532.cpp
 * @brief Algorithm module 1532
 */
#include "compress1532/algo_1532.h"
QVector<double> algo_1532::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
