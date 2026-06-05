/**
 * @file algo_1762.cpp
 * @brief Algorithm module 1762
 */
#include "poly1762/algo_1762.h"
QVector<double> algo_1762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
