/**
 * @file algo_972.cpp
 * @brief Algorithm module 972
 */
#include "compress972/algo_972.h"
QVector<double> algo_972::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
