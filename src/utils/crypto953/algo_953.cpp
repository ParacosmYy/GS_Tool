/**
 * @file algo_953.cpp
 * @brief Algorithm module 953
 */
#include "crypto953/algo_953.h"
QVector<double> algo_953::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
