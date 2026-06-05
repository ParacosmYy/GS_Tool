/**
 * @file algo_4249.cpp
 */
#include "code4249/algo_4249.h"
QVector<double> algo_4249::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
