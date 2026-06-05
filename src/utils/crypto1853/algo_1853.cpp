/**
 * @file algo_1853.cpp
 * @brief Algorithm module 1853
 */
#include "crypto1853/algo_1853.h"
QVector<double> algo_1853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
