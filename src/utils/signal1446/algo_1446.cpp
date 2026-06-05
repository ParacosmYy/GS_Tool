/**
 * @file algo_1446.cpp
 * @brief Algorithm module 1446
 */
#include "signal1446/algo_1446.h"
QVector<double> algo_1446::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
