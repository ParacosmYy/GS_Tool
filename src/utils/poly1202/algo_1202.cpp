/**
 * @file algo_1202.cpp
 * @brief Algorithm module 1202
 */
#include "poly1202/algo_1202.h"
QVector<double> algo_1202::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
