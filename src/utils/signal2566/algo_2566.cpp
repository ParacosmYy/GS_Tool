/**
 * @file algo_2566.cpp
 * @brief Algorithm module 2566
 */
#include "signal2566/algo_2566.h"
QVector<double> algo_2566::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
