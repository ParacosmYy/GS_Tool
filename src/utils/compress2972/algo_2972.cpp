/**
 * @file algo_2972.cpp
 */
#include "compress2972/algo_2972.h"
QVector<double> algo_2972::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
