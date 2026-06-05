/**
 * @file algo_5992.cpp
 */
#include "compress5992/algo_5992.h"
QVector<double> algo_5992::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
