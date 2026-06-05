/**
 * @file algo_1713.cpp
 * @brief Algorithm module 1713
 */
#include "crypto1713/algo_1713.h"
QVector<double> algo_1713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
