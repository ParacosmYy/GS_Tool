/**
 * @file algo_1321.cpp
 * @brief Algorithm module 1321
 */
#include "interp1321/algo_1321.h"
QVector<double> algo_1321::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
