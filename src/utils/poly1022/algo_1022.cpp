/**
 * @file algo_1022.cpp
 * @brief Algorithm module 1022
 */
#include "poly1022/algo_1022.h"
QVector<double> algo_1022::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
