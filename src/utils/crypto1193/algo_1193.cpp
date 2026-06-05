/**
 * @file algo_1193.cpp
 * @brief Algorithm module 1193
 */
#include "crypto1193/algo_1193.h"
QVector<double> algo_1193::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
