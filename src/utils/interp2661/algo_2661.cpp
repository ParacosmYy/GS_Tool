/**
 * @file algo_2661.cpp
 * @brief Algorithm module 2661
 */
#include "interp2661/algo_2661.h"
QVector<double> algo_2661::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
