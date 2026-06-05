/**
 * @file algo_1661.cpp
 * @brief Algorithm module 1661
 */
#include "interp1661/algo_1661.h"
QVector<double> algo_1661::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
