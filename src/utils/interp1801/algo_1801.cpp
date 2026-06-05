/**
 * @file algo_1801.cpp
 * @brief Algorithm module 1801
 */
#include "interp1801/algo_1801.h"
QVector<double> algo_1801::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
