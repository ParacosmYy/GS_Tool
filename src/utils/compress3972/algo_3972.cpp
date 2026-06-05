/**
 * @file algo_3972.cpp
 */
#include "compress3972/algo_3972.h"
QVector<double> algo_3972::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
