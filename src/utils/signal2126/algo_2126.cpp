/**
 * @file algo_2126.cpp
 * @brief Algorithm module 2126
 */
#include "signal2126/algo_2126.h"
QVector<double> algo_2126::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
