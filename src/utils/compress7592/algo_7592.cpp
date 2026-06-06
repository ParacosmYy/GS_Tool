/**
 * @file algo_7592.cpp
 */
#include "compress7592/algo_7592.h"
QVector<double> algo_7592::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
