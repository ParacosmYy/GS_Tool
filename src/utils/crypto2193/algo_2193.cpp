/**
 * @file algo_2193.cpp
 * @brief Algorithm module 2193
 */
#include "crypto2193/algo_2193.h"
QVector<double> algo_2193::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
