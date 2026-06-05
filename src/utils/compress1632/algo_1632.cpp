/**
 * @file algo_1632.cpp
 * @brief Algorithm module 1632
 */
#include "compress1632/algo_1632.h"
QVector<double> algo_1632::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
