/**
 * @file algo_1922.cpp
 * @brief Algorithm module 1922
 */
#include "poly1922/algo_1922.h"
QVector<double> algo_1922::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
