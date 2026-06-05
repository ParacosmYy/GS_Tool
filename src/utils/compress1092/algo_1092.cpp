/**
 * @file algo_1092.cpp
 * @brief Algorithm module 1092
 */
#include "compress1092/algo_1092.h"
QVector<double> algo_1092::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
