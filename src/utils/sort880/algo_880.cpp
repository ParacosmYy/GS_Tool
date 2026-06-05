/**
 * @file algo_880.cpp
 * @brief Algorithm module 880
 */
#include "sort880/algo_880.h"
QVector<double> algo_880::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
