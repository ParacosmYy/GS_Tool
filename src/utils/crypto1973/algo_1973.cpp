/**
 * @file algo_1973.cpp
 * @brief Algorithm module 1973
 */
#include "crypto1973/algo_1973.h"
QVector<double> algo_1973::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
