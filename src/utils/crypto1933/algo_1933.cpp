/**
 * @file algo_1933.cpp
 * @brief Algorithm module 1933
 */
#include "crypto1933/algo_1933.h"
QVector<double> algo_1933::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
