/**
 * @file algo_1572.cpp
 * @brief Algorithm module 1572
 */
#include "compress1572/algo_1572.h"
QVector<double> algo_1572::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
