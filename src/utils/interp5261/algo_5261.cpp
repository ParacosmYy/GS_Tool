/**
 * @file algo_5261.cpp
 */
#include "interp5261/algo_5261.h"
QVector<double> algo_5261::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
