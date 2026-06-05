/**
 * @file algo_2541.cpp
 * @brief Algorithm module 2541
 */
#include "interp2541/algo_2541.h"
QVector<double> algo_2541::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
