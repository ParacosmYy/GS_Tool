/**
 * @file algo_1566.cpp
 * @brief Algorithm module 1566
 */
#include "signal1566/algo_1566.h"
QVector<double> algo_1566::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
