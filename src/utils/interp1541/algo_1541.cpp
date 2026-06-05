/**
 * @file algo_1541.cpp
 * @brief Algorithm module 1541
 */
#include "interp1541/algo_1541.h"
QVector<double> algo_1541::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
