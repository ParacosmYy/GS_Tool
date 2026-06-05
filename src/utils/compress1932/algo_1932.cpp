/**
 * @file algo_1932.cpp
 * @brief Algorithm module 1932
 */
#include "compress1932/algo_1932.h"
QVector<double> algo_1932::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
